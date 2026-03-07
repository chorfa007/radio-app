#include "database/Database.h"
#include "player/Player.h"
#include "cli/CLI.h"

#include <iostream>
#include <filesystem>
#include <cstdlib>

int main() {
    // ── locate / create DB in user home ───────────────────────────────────
    const char* home = std::getenv("HOME");
    std::string dbDir = home ? std::string(home) + "/.local/share/radio-app" : ".";
    std::filesystem::create_directories(dbDir);
    std::string dbPath = dbDir + "/radios.db";

    try {
        Database db(dbPath);
        db.initSchema();
        if (!db.isSeeded()) {
            db.seedData();
        }

        Player player;
        CLI    cli(db, player);
        cli.run();

    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
