#pragma once
#include "../database/Database.h"
#include "../player/Player.h"
#include <string>
#include <optional>

class CLI {
public:
    CLI(Database& db, Player& player);
    void run();

private:
    Database& m_db;
    Player&   m_player;

    // ── screens ─────────────────────────────────────────────────────────
    void showMainMenu();
    void showCountryMenu();
    void showRadioMenu(const Country& country);
    void showNowPlaying(const Radio& radio);

    // ── UI helpers ──────────────────────────────────────────────────────
    void clearScreen() const;
    void printHeader() const;
    void printFooter(const std::string& hint = "") const;
    void printNowPlayingBar() const;
    void printSeparator(const std::string& ch = "\xe2\x94\x80", int width = 60) const;
    void printBox(const std::string& text, int width = 60) const;
    void printError(const std::string& msg) const;
    void printSuccess(const std::string& msg) const;

    std::string readLine(const std::string& prompt) const;
    std::optional<int> readInt(const std::string& prompt,
                               int min, int max) const;
    void waitForEnter(const std::string& msg = "Press Enter to continue...") const;
};
