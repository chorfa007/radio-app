#include "CLI.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <thread>
#include <chrono>
#include <algorithm>

// ─── ANSI colour codes ──────────────────────────────────────────────────────

namespace Color {
    constexpr auto Reset   = "\033[0m";
    constexpr auto Bold    = "\033[1m";
    constexpr auto Dim     = "\033[2m";

    // Foreground
    constexpr auto Black   = "\033[30m";
    constexpr auto Yellow  = "\033[33m";
    constexpr auto Cyan    = "\033[36m";
    constexpr auto White   = "\033[37m";

    // Bright foreground
    constexpr auto BrBlack   = "\033[90m";
    constexpr auto BrRed     = "\033[91m";
    constexpr auto BrGreen   = "\033[92m";
    constexpr auto BrYellow  = "\033[93m";
    constexpr auto BrMagenta = "\033[95m";
    constexpr auto BrCyan    = "\033[96m";
    constexpr auto BrWhite   = "\033[97m";

    // Background
    constexpr auto BgBlue    = "\033[44m";
    constexpr auto BgMagenta = "\033[45m";
    constexpr auto BgCyan    = "\033[46m";
}

// ─── constructor ────────────────────────────────────────────────────────────

CLI::CLI(Database& db, Player& player)
    : m_db(db), m_player(player) {}

// ─── run loop ────────────────────────────────────────────────────────────────

void CLI::run() {
    while (true) {
        showMainMenu();
    }
}

// ─── screens ────────────────────────────────────────────────────────────────

void CLI::showMainMenu() {
    clearScreen();
    printHeader();
    printNowPlayingBar();

    std::cout << "\n"
              << Color::BrWhite << Color::Bold
              << "  Main Menu\n"
              << Color::Reset;
    printSeparator();

    std::cout << "\n"
              << Color::BrCyan   << "  [1]" << Color::Reset << "  Browse by Country\n"
              << Color::BrYellow << "  [2]" << Color::Reset << "  Stop Playback\n"
              << Color::BrRed    << "  [0]" << Color::Reset << "  Quit\n\n";

    printSeparator();
    auto choice = readInt("", 0, 2);
    if (!choice) return;

    switch (*choice) {
        case 1: showCountryMenu(); break;
        case 2:
            m_player.stop();
            printSuccess("  Playback stopped.");
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
            break;
        case 0:
            m_player.stop();
            clearScreen();
            std::cout << "\n  " << Color::BrCyan << "Thanks for listening!"
                      << Color::Reset << "\n\n";
            std::exit(0);
    }
}

void CLI::showCountryMenu() {
    auto countries = m_db.getCountries();

    clearScreen();
    printHeader();
    printNowPlayingBar();

    std::cout << "\n"
              << Color::BrWhite << Color::Bold
              << "  Select a Country\n"
              << Color::Reset;
    printSeparator();
    std::cout << "\n";

    for (int i = 0; i < static_cast<int>(countries.size()); ++i) {
        const auto& c = countries[i];
        std::cout << Color::BrCyan << "  [" << (i+1) << "]" << Color::Reset
                  << "  " << c.flag << "  "
                  << Color::BrWhite << c.name
                  << Color::Reset << "\n";
    }
    std::cout << Color::BrBlack << "\n  [0]" << Color::Reset << "  Back\n\n";
    printSeparator();

    auto choice = readInt("", 0, static_cast<int>(countries.size()));
    if (!choice || *choice == 0) return;

    showRadioMenu(countries[*choice - 1]);
}

void CLI::showRadioMenu(const Country& country) {
    auto radios = m_db.getRadiosByCountry(country.id);

    while (true) {
        clearScreen();
        printHeader();
        printNowPlayingBar();

        std::cout << "\n"
                  << Color::BrWhite << Color::Bold
                  << "  " << country.flag << "  " << country.name << " — Stations\n"
                  << Color::Reset;
        printSeparator();
        std::cout << "\n";

        for (int i = 0; i < static_cast<int>(radios.size()); ++i) {
            const auto& r = radios[i];
            std::string active = (m_player.isPlaying() && m_player.currentStationName() == r.name)
                                 ? (std::string(Color::BrGreen) + " ▶ " + Color::Reset)
                                 : "   ";
            std::cout << active
                      << Color::BrCyan << "[" << (i+1) << "]" << Color::Reset
                      << "  " << Color::BrWhite << std::left << std::setw(22) << r.name
                      << Color::Reset
                      << Color::Dim << "  " << r.genre
                      << Color::Reset << "\n";

            // Show description indented
            if (!r.description.empty()) {
                std::cout << Color::BrBlack
                          << "       " << r.description
                          << Color::Reset << "\n";
            }
            std::cout << "\n";
        }

        std::cout << Color::BrBlack << "  [0]" << Color::Reset << "  Back\n\n";
        printSeparator();
        std::cout << Color::Dim
                  << "  Choose a station to start streaming.\n"
                  << Color::Reset;

        auto choice = readInt("", 0, static_cast<int>(radios.size()));
        if (!choice || *choice == 0) return;

        const Radio& selected = radios[*choice - 1];
        showNowPlaying(selected);
    }
}

void CLI::showNowPlaying(const Radio& radio) {
    m_player.stop();

    clearScreen();
    printHeader();

    std::cout << "\n";
    printBox("  Connecting to stream...", 60);
    std::cout << "\n";

    bool ok = m_player.play(radio);
    if (!ok) {
        printError("  Could not start player. Is mpv/vlc/ffplay installed?");
        waitForEnter();
        return;
    }

    // Brief pause to let the stream connect
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    clearScreen();
    printHeader();

    std::cout << "\n"
              << Color::BgCyan << Color::Black << Color::Bold
              << "  NOW PLAYING                                              "
              << Color::Reset << "\n\n"
              << Color::BrGreen << "  ▶  " << Color::BrWhite << Color::Bold
              << radio.name << Color::Reset << "\n\n"
              << Color::Dim << "  Genre  :  " << Color::Reset << Color::Yellow
              << radio.genre << Color::Reset << "\n"
              << Color::Dim << "  Info   :  " << Color::Reset
              << radio.description << "\n"
              << Color::Dim << "  URL    :  " << Color::Reset << Color::BrBlack
              << radio.streamUrl << Color::Reset << "\n\n";

    printSeparator("\xe2\x95\x90");  // ═

    std::cout << "\n"
              << Color::BrCyan   << "  [s]" << Color::Reset << "  Stop\n"
              << Color::BrYellow << "  [b]" << Color::Reset << "  Back to station list\n"
              << Color::BrRed    << "  [q]" << Color::Reset << "  Quit\n\n";

    printSeparator();

    // Interactive now-playing loop — keep refreshing status
    while (true) {
        std::cout << Color::BrMagenta << "  > " << Color::Reset << std::flush;
        std::string line;
        std::getline(std::cin, line);

        if (line == "s" || line == "S") {
            m_player.stop();
            printSuccess("  Stopped.");
            std::this_thread::sleep_for(std::chrono::milliseconds(600));
            return;
        } else if (line == "b" || line == "B") {
            return; // keep playing, go back
        } else if (line == "q" || line == "Q") {
            m_player.stop();
            clearScreen();
            std::cout << "\n  " << Color::BrCyan << "Thanks for listening!"
                      << Color::Reset << "\n\n";
            std::exit(0);
        } else {
            printError("  Unknown command. Use s / b / q.");
        }
    }
}

// ─── UI helpers ─────────────────────────────────────────────────────────────

void CLI::clearScreen() const {
    std::cout << "\033[2J\033[H" << std::flush;
}

void CLI::printHeader() const {
    std::cout << "\n"
              << Color::BgMagenta << Color::Bold << Color::White
              << "                                                            \n"
              << "      Radio Streaming App  |  TN  BE  GB                   \n"
              << "                                                            "
              << Color::Reset << "\n";
}

void CLI::printNowPlayingBar() const {
    if (m_player.isPlaying()) {
        std::cout << Color::BgBlue << Color::BrWhite
                  << "  ▶  Now playing: " << m_player.currentStationName()
                  << std::string(std::max(0, 41 - (int)m_player.currentStationName().size()), ' ')
                  << Color::Reset << "\n";
    } else {
        std::cout << Color::BrBlack
                  << "  ○  Not playing"
                  << Color::Reset << "\n";
    }
}

void CLI::printSeparator(const std::string& ch, int width) const {
    std::cout << Color::BrBlack << "  ";
    for (int i = 0; i < width - 2; ++i) std::cout << ch;
    std::cout << Color::Reset << "\n";
}

void CLI::printBox(const std::string& text, int width) const {
    std::string border;
    for (int i = 0; i < width; ++i) border += "\xe2\x94\x80";  // ─
    std::cout << Color::Cyan << "  \xe2\x94\x8c" << border << "\xe2\x94\x90\n"
              << "  \xe2\x94\x82 " << Color::BrWhite << std::left
              << std::setw(width - 1) << text
              << Color::Cyan << "\xe2\x94\x82\n"
              << "  \xe2\x94\x94" << border << "\xe2\x94\x98"
              << Color::Reset << "\n";
}

void CLI::printError(const std::string& msg) const {
    std::cout << Color::BrRed << msg << Color::Reset << "\n";
}

void CLI::printSuccess(const std::string& msg) const {
    std::cout << Color::BrGreen << msg << Color::Reset << "\n";
}

std::string CLI::readLine(const std::string& prompt) const {
    if (!prompt.empty())
        std::cout << Color::BrMagenta << prompt << Color::Reset << std::flush;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

std::optional<int> CLI::readInt(const std::string& /*prompt*/, int minVal, int maxVal) const {
    while (true) {
        std::cout << Color::BrMagenta << "  > " << Color::Reset << std::flush;
        std::string line;
        if (!std::getline(std::cin, line)) return std::nullopt; // EOF

        // trim
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        try {
            int val = std::stoi(line);
            if (val >= minVal && val <= maxVal) return val;
            printError("  Out of range. Enter a number between "
                       + std::to_string(minVal) + " and " + std::to_string(maxVal) + ".");
        } catch (...) {
            printError("  Invalid input. Please enter a number.");
        }
    }
}

void CLI::waitForEnter(const std::string& msg) const {
    std::cout << Color::Dim << "\n  " << msg << Color::Reset << std::flush;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
