#pragma once
#include <QObject>
#include <QString>
#include <QProcess>
#include <QLocalSocket>
#include <QTimer>

class PlayerController : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool    playing     READ isPlaying     NOTIFY playingChanged)
    Q_PROPERTY(bool    loading     READ isLoading     NOTIFY loadingChanged)
    Q_PROPERTY(QString stationName READ stationName   NOTIFY stationChanged)
    Q_PROPERTY(QString genre       READ genre         NOTIFY stationChanged)
    Q_PROPERTY(QString description READ description   NOTIFY stationChanged)
    Q_PROPERTY(float   volume      READ volume  WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(QString errorString READ errorString   NOTIFY errorOccurred)

public:
    explicit PlayerController(QObject* parent = nullptr);
    ~PlayerController();

    bool    isPlaying()   const { return m_playing;     }
    bool    isLoading()   const { return m_loading;     }
    QString stationName() const { return m_stationName; }
    QString genre()       const { return m_genre;       }
    QString description() const { return m_description; }
    float   volume()      const { return m_volume;      }
    QString errorString() const { return m_errorString; }

    Q_INVOKABLE void play(const QString& url,
                          const QString& name,
                          const QString& genre,
                          const QString& description);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void setVolume(float v);

signals:
    void playingChanged();
    void loadingChanged();
    void stationChanged();
    void volumeChanged();
    void errorOccurred();

private slots:
    void onOutput();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void tryConnectIpc();
    void onIpcConnected();

private:
    void killProcess();
    void sendIpcCommand(const QByteArray& jsonCmd);

    QProcess*     m_process{nullptr};
    QLocalSocket* m_ipc{nullptr};
    QTimer*       m_ipcRetryTimer{nullptr};

    QString m_stationName;
    QString m_genre;
    QString m_description;
    QString m_errorString;
    float   m_volume{0.80f};
    bool    m_playing{false};
    bool    m_loading{false};

    static constexpr const char* k_ipcPath = "/tmp/radio-mpv-gui.sock";
};
