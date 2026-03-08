#include "PlayerController.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QTimer>

PlayerController::PlayerController(QObject* parent)
    : QObject(parent)
    , m_ipcRetryTimer(new QTimer(this))
{
    m_ipcRetryTimer->setInterval(400);
    connect(m_ipcRetryTimer, &QTimer::timeout, this, &PlayerController::tryConnectIpc);
}

PlayerController::~PlayerController() {
    killProcess();
}

// ─── public API ─────────────────────────────────────────────────────────────

void PlayerController::play(const QString& url,
                            const QString& name,
                            const QString& genre,
                            const QString& description) {
    killProcess();              // safe: signals are disconnected before any delete
    QFile::remove(k_ipcPath);  // remove stale socket

    m_stationName  = name;
    m_genre        = genre;
    m_description  = description;
    m_errorString.clear();
    m_errorStation.clear();
    m_loading = true;
    m_playing = false;
    emit stationChanged();
    emit loadingChanged();
    emit playingChanged();

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::readyRead,
            this, &PlayerController::onOutput);
    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &PlayerController::onProcessFinished);

    QStringList args = {
        "--no-video",
        "--no-ytdl",
        "--no-terminal",
        "--really-quiet",
        "--msg-level=all=status",
        QString("--volume=%1").arg(qRound(m_volume * 100)),
        QString("--input-ipc-server=%1").arg(k_ipcPath),
        url
    };

    m_process->start("mpv", args);

    if (!m_process->waitForStarted(3000)) {
        m_errorString = "Failed to start mpv. Is it installed?";
        m_loading     = false;
        emit errorOccurred();
        emit loadingChanged();
        // disconnect before cleanup to avoid reentrancy
        m_process->disconnect();
        m_process->deleteLater();
        m_process = nullptr;
        return;
    }

    m_ipcRetryTimer->start();
}

void PlayerController::stop() {
    killProcess();
    m_stationName.clear();
    m_genre.clear();
    m_description.clear();
    m_errorStation.clear();
    m_playing = false;
    m_loading = false;
    emit stationChanged();
    emit playingChanged();
    emit loadingChanged();
}

void PlayerController::setVolume(float v) {
    m_volume = qBound(0.0f, v, 1.0f);
    emit volumeChanged();

    if (m_ipc && m_ipc->state() == QLocalSocket::ConnectedState) {
        QJsonObject cmd;
        cmd["command"] = QJsonArray{ "set_property", "volume",
                                     static_cast<int>(m_volume * 100) };
        sendIpcCommand(QJsonDocument(cmd).toJson(QJsonDocument::Compact));
    }
}

// ─── private slots ───────────────────────────────────────────────────────────

void PlayerController::onOutput() {
    if (!m_process) return;
    const QByteArray data = m_process->readAll();

    if (m_loading && data.contains("AO:")) {
        m_loading = false;
        m_playing = true;
        emit loadingChanged();
        emit playingChanged();
    }
}

void PlayerController::onProcessFinished(int /*exitCode*/,
                                          QProcess::ExitStatus /*status*/) {
    // Only handle if this signal came from our current process
    QProcess* proc = qobject_cast<QProcess*>(sender());
    if (proc != m_process) return;

    m_ipcRetryTimer->stop();

    // Clear the member pointer BEFORE deleteLater so any re-entrant
    // call to killProcess() sees nullptr and skips the delete.
    m_process = nullptr;
    proc->deleteLater();

    // Process died while still buffering = stream unreachable
    if (m_loading) {
        m_errorStation = m_stationName;
        emit streamUnavailable();
    }

    m_playing = false;
    m_loading = false;
    emit playingChanged();
    emit loadingChanged();
}

void PlayerController::tryConnectIpc() {
    if (!QFile::exists(k_ipcPath)) return;

    m_ipcRetryTimer->stop();

    delete m_ipc;
    m_ipc = new QLocalSocket(this);
    connect(m_ipc, &QLocalSocket::connected,
            this, &PlayerController::onIpcConnected);

    m_ipc->connectToServer(k_ipcPath);
}

void PlayerController::onIpcConnected() {
    setVolume(m_volume);
}

// ─── private helpers ─────────────────────────────────────────────────────────

void PlayerController::killProcess() {
    m_ipcRetryTimer->stop();

    if (m_ipc) {
        m_ipc->disconnect();
        m_ipc->deleteLater();
        m_ipc = nullptr;
    }

    if (m_process) {
        // Disconnect ALL signals FIRST — prevents onProcessFinished (or any
        // other slot) from firing during the termination sequence and
        // causing reentrancy / double-delete.
        m_process->disconnect();

        m_process->terminate();

        // Do NOT call waitForFinished() — it pumps the event loop and
        // can cause reentrant calls back into this object.
        // Instead, schedule a deferred kill so the OS can clean up.
        QProcess* p = m_process;
        m_process = nullptr;        // safe for any re-entrant paths

        QTimer::singleShot(1200, p, [p]() {
            if (p->state() != QProcess::NotRunning)
                p->kill();
            p->deleteLater();
        });
    }

    QFile::remove(k_ipcPath);
}

void PlayerController::sendIpcCommand(const QByteArray& jsonCmd) {
    if (!m_ipc) return;
    m_ipc->write(jsonCmd + '\n');
    m_ipc->flush();
}
