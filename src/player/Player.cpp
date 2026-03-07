#include "Player.h"
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstdlib>

// ─── helpers ────────────────────────────────────────────────────────────────

static std::string whichBinary(const std::string& name) {
    // Search common install locations + PATH via popen
    std::string cmd = "which " + name + " 2>/dev/null";
    char buf[256];
    FILE* f = popen(cmd.c_str(), "r");
    if (!f) return {};
    std::string result;
    if (fgets(buf, sizeof(buf), f)) {
        result = buf;
        while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
            result.pop_back();
    }
    pclose(f);
    return result;
}

// ─── Player ─────────────────────────────────────────────────────────────────

Player::Player() = default;

Player::~Player() {
    stop();
}

std::optional<std::string> Player::findPlayerBinary() const {
    for (const char* name : { "mpv", "vlc", "ffplay", "mplayer" }) {
        std::string path = whichBinary(name);
        if (!path.empty()) return path;
    }
    return std::nullopt;
}

bool Player::play(const Radio& radio) {
    stop(); // stop any current stream

    auto bin = findPlayerBinary();
    if (!bin) return false;

    const std::string& player = *bin;
    const std::string& url    = radio.streamUrl;

    pid_t pid = fork();
    if (pid < 0) return false;

    if (pid == 0) {
        // ── child process ─────────────────────────────────────────────────
        // Redirect stdout/stderr to /dev/null so player output doesn't clutter CLI
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }

        if (player.find("mpv") != std::string::npos) {
            execlp(player.c_str(), "mpv",
                "--no-video",
                "--no-terminal",
                "--no-ytdl",   // bypass yt-dlp hook; use direct HTTP for icecast streams
                "--quiet",
                url.c_str(),
                nullptr);
        } else if (player.find("vlc") != std::string::npos) {
            execlp(player.c_str(), "vlc",
                "--intf", "dummy",
                "--no-video",
                "--quiet",
                url.c_str(),
                "vlc://quit",
                nullptr);
        } else if (player.find("ffplay") != std::string::npos) {
            execlp(player.c_str(), "ffplay",
                "-nodisp",
                "-loglevel", "quiet",
                url.c_str(),
                nullptr);
        } else {
            execlp(player.c_str(), player.c_str(), url.c_str(), nullptr);
        }
        _exit(1); // exec failed
    }

    // parent
    m_pid         = pid;
    m_currentName = radio.name;
    m_currentUrl  = radio.streamUrl;
    return true;
}

void Player::stop() {
    if (m_pid > 0) {
        kill(m_pid, SIGTERM);
        waitpid(m_pid, nullptr, 0);
        m_pid         = -1;
        m_currentName.clear();
        m_currentUrl.clear();
    }
}

bool Player::isPlaying() const {
    if (m_pid <= 0) return false;
    // check if child is still alive
    int status = 0;
    pid_t r = waitpid(m_pid, &status, WNOHANG);
    if (r == 0) return true;   // still running
    if (r == m_pid) {
        const_cast<Player*>(this)->m_pid = -1; // died on its own
    }
    return false;
}
