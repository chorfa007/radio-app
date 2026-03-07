#pragma once
#include "../models/Radio.h"
#include <string>
#include <optional>
#include <sys/types.h>

class Player {
public:
    Player();
    ~Player();

    Player(const Player&)            = delete;
    Player& operator=(const Player&) = delete;

    bool        play(const Radio& radio);
    void        stop();
    bool        isPlaying() const;
    std::string currentStationName() const { return m_currentName; }
    std::string currentUrl()         const { return m_currentUrl;  }

private:
    pid_t       m_pid{-1};
    std::string m_currentName;
    std::string m_currentUrl;

    std::optional<std::string> findPlayerBinary() const;
};
