#include "Database.h"
#include <stdexcept>
#include <functional>

// ─── helpers ────────────────────────────────────────────────────────────────

static std::string col(sqlite3_stmt* s, int i) {
    const char* v = reinterpret_cast<const char*>(sqlite3_column_text(s, i));
    return v ? v : "";
}

// ─── ctor / dtor ────────────────────────────────────────────────────────────

Database::Database(const std::string& path) {
    if (sqlite3_open(path.c_str(), &m_db) != SQLITE_OK) {
        throw std::runtime_error(std::string("Cannot open DB: ") + sqlite3_errmsg(m_db));
    }
    exec("PRAGMA foreign_keys = ON;");
    exec("PRAGMA journal_mode = WAL;");
}

Database::~Database() {
    if (m_db) sqlite3_close(m_db);
}

// ─── private helpers ────────────────────────────────────────────────────────

void Database::exec(const std::string& sql) const {
    char* err = nullptr;
    if (sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown error";
        sqlite3_free(err);
        throw std::runtime_error("SQL error: " + msg);
    }
}

// ─── schema ─────────────────────────────────────────────────────────────────

void Database::initSchema() {
    exec(R"(
        CREATE TABLE IF NOT EXISTS countries (
            id   INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            code TEXT NOT NULL UNIQUE,
            flag TEXT NOT NULL DEFAULT ''
        );

        CREATE TABLE IF NOT EXISTS radios (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            name        TEXT NOT NULL,
            stream_url  TEXT NOT NULL,
            genre       TEXT NOT NULL DEFAULT '',
            description TEXT NOT NULL DEFAULT '',
            logo_url    TEXT NOT NULL DEFAULT '',
            country_id  INTEGER NOT NULL,
            FOREIGN KEY (country_id) REFERENCES countries(id)
        );
    )");
}

bool Database::isSeeded() const {
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(m_db, "SELECT COUNT(*) FROM countries;", -1, &stmt, nullptr);
    sqlite3_step(stmt);
    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return count > 0;
}

// ─── seed ───────────────────────────────────────────────────────────────────

void Database::seedData() {
    exec("BEGIN TRANSACTION;");

    // ── Countries ──────────────────────────────────────────────────────────
    exec(R"(
        INSERT OR IGNORE INTO countries (name, code, flag) VALUES
            ('Tunisia',        'TN', '🇹🇳'),
            ('Belgium',        'BE', '🇧🇪'),
            ('United Kingdom', 'GB', '🇬🇧'),
            ('France',         'FR', '🇫🇷');
    )");

    // Grab IDs
    auto getId = [&](const std::string& code) -> int {
        sqlite3_stmt* s = nullptr;
        sqlite3_prepare_v2(m_db,
            "SELECT id FROM countries WHERE code = ?;", -1, &s, nullptr);
        sqlite3_bind_text(s, 1, code.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(s);
        int id = sqlite3_column_int(s, 0);
        sqlite3_finalize(s);
        return id;
    };

    int tn = getId("TN");
    int be = getId("BE");
    int gb = getId("GB");
    int fr = getId("FR");

    // ── Tunisia ───────────────────────────────────────────────────────────
    struct R { std::string name, url, genre, desc, logo; int cid; };
    // All stream URLs verified working with mpv --no-ytdl on 2026-03-07
    // Logo URLs use Google's favicon service (sz=128) for reliable cross-origin icons
    static const std::string gf = "https://icons.duckduckgo.com/ip3/";
    std::vector<R> rows = {
        // ── Tunisia ──────────────────────────────────────────────────────────
        { "Mosaique FM",          "https://radio.mosaiquefm.net/mosalive",                  "News / Talk",    "Tunisia's most popular radio",          gf+"mosaiquefm.net.ico",      tn },
        { "Mosaique Tounsi",      "https://radio.mosaiquefm.net/mosatounsi",                "Arabic Pop",     "Tunisian & Arabic pop music",           gf+"mosaiquefm.net.ico",      tn },
        { "Mosaique Gold",        "https://radio.mosaiquefm.net/mosagold",                  "Oldies",         "Classic Arabic & international hits",   gf+"mosaiquefm.net.ico",      tn },
        { "Express FM",           "https://expressfm.ice.infomaniak.ch/expressfm-64.mp3",   "News / Music",   "Express FM news & entertainment",       gf+"expressfm.com.tn.ico",    tn },
        { "Jawhara FM",           "https://streaming2.toutech.net/jawharafm",               "Music",          "Top hits & Arabic pop music",           gf+"jawharafm.net.ico",       tn },
        { "Diwan FM",             "https://streaming.diwanfm.net/stream",                   "Arabic Music",   "Arabic classical & modern music",       gf+"diwanfm.net.ico",         tn },
        { "Ambiance FM",          "https://stream.zeno.fm/0rehsamc9xxtv",                   "Pop / Dance",    "Pop, dance & variety hits",             gf+"ambiance.fm.ico",         tn },
        { "Sfax FM",              "http://rtstream.tanitweb.com/sfax",                      "Regional",       "Regional radio from Sfax",              gf+"tanitweb.com.ico",        tn },
        { "Radio Tunisie Culturelle","http://rtstream.tanitweb.com/culturelle",             "Culture",        "Tunisian national cultural radio",      gf+"tanitweb.com.ico",        tn },
        { "Radio Kef",            "http://rtstream.tanitweb.com/kef",                       "Regional",       "Regional radio from Le Kef",            gf+"tanitweb.com.ico",        tn },

        // ── Belgium ──────────────────────────────────────────────────────────
        { "Radio 1 (VRT)",        "https://icecast.vrtcdn.be/radio1-high.mp3",              "News / Culture", "Flemish public radio — news & culture", gf+"vrt.be.ico",              be },
        { "Studio Brussel",       "https://icecast.vrtcdn.be/stubru-high.mp3",              "Alternative",    "Alternative, indie & dance music",      gf+"stubru.be.ico",           be },
        { "MNM",                  "https://icecast.vrtcdn.be/mnm-high.mp3",                 "Pop / Top 40",   "Belgium's biggest hit music station",   gf+"mnm.be.ico",              be },
        { "MNM Hits",             "https://icecast.vrtcdn.be/mnm_hits-high.mp3",            "Pop Hits",       "Non-stop chart hits",                   gf+"mnm.be.ico",              be },
        { "Klara",                "https://icecast.vrtcdn.be/klara-high.mp3",               "Classical",      "Classical music & culture",             gf+"klara.be.ico",            be },
        { "Classic 21 Metal",     "https://radios.rtbf.be/wr-c21-metal-128.mp3",            "Metal / Rock",   "RTBF hard rock & metal",               gf+"rtbf.be.ico",             be },

        // ── United Kingdom ───────────────────────────────────────────────────
        { "Capital FM",           "https://media-ice.musicradio.com/CapitalMP3",            "Pop / Top 40",   "UK's biggest hit music network",        gf+"capitalfm.com.ico",       gb },
        { "Heart 80s",            "https://media-ice.musicradio.com/Heart80sMP3",           "80s Classics",   "The best music of the 80s",             gf+"heart.co.uk.ico",         gb },
        { "Classic FM",           "https://media-ice.musicradio.com/ClassicFMMP3",          "Classical",      "UK's most popular classical station",   gf+"classicfm.com.ico",       gb },
        { "Gold",                 "https://media-ice.musicradio.com/GoldMP3",               "Oldies",         "Greatest hits from 60s–90s",            gf+"gold.co.uk.ico",          gb },
        { "BBC World Service",    "http://stream.live.vc.bbcmedia.co.uk/bbc_world_service", "News",           "BBC global news & current affairs",     gf+"bbc.co.uk.ico",           gb },
        { "Radio Paradise",       "https://stream-uk1.radioparadise.com/mp3-128",           "Eclectic Rock",  "Listener-supported, no ads",            gf+"radioparadise.com.ico",   gb },
        { "SomaFM Groove Salad",  "https://ice3.somafm.com/groovesalad-128-mp3",            "Chill / Ambient","A nicely chilled plate of ambient",    gf+"somafm.com.ico",          gb },
        { "SomaFM Lush",          "https://ice3.somafm.com/lush-128-mp3",                   "Chillout",       "Sensuous, lush vocals & moods",         gf+"somafm.com.ico",          gb },
        { "SomaFM 70s",           "https://ice3.somafm.com/seventies-128-mp3",              "70s Hits",       "All the classics from the 70s",         gf+"somafm.com.ico",          gb },
        { "Cam FM",               "https://stream.camfm.co.uk/camfm",                       "Variety",        "Cambridge University radio station",    gf+"camfm.co.uk.ico",         gb },

        // ── France ──────────────────────────────────────────────────────────
        { "France Inter",         "https://icecast.radiofrance.fr/franceinter-midfi.mp3",   "News / Culture", "France's flagship public radio",        gf+"franceinter.fr.ico",      fr },
        { "France Info",          "https://icecast.radiofrance.fr/franceinfo-midfi.mp3",    "News",           "24/7 news & current affairs",           gf+"francetvinfo.fr.ico",     fr },
        { "France Musique",       "https://icecast.radiofrance.fr/francemusique-midfi.mp3", "Classical",      "Classical & jazz from Radio France",    gf+"francemusique.fr.ico",    fr },
        { "France Culture",       "https://icecast.radiofrance.fr/franceculture-midfi.mp3", "Culture",        "Ideas, documentaries & debates",        gf+"franceculture.fr.ico",    fr },
        { "FIP",                  "https://icecast.radiofrance.fr/fip-midfi.mp3",           "Eclectic",       "Eclectic music — no ads, no talk",      gf+"fip.fr.ico",              fr },
        { "Sud Radio",            "https://ice.creacast.com/sudradio",                      "News / Talk",    "News, talk & opinion from the South",   gf+"sudradio.fr.ico",         fr },
        { "Jazz Radio",           "https://jazz-wr01.ice.infomaniak.ch/jazz-wr01-128.mp3",  "Jazz",           "French jazz radio — all styles",        gf+"jazzradio.fr.ico",        fr },
        { "Jazz Manouche",        "https://jazz-wr02.ice.infomaniak.ch/jazz-wr02-128.mp3",  "Jazz Manouche",  "Gypsy jazz & jazz manouche",            gf+"jazzradio.fr.ico",        fr },
        { "Voltage 80s",          "http://voltage80s.ice.infomaniak.ch/voltage80s-128.mp3", "80s Hits",       "Non-stop 80s hits in French & English", gf+"voltage.fr.ico",          fr },
    };

    sqlite3_stmt* ins = nullptr;
    sqlite3_prepare_v2(m_db,
        "INSERT OR IGNORE INTO radios (name, stream_url, genre, description, logo_url, country_id) "
        "VALUES (?, ?, ?, ?, ?, ?);",
        -1, &ins, nullptr);

    for (auto& r : rows) {
        sqlite3_bind_text(ins, 1, r.name.c_str(),  -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(ins, 2, r.url.c_str(),   -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(ins, 3, r.genre.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(ins, 4, r.desc.c_str(),  -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(ins, 5, r.logo.c_str(),  -1, SQLITE_TRANSIENT);
        sqlite3_bind_int (ins, 6, r.cid);
        sqlite3_step(ins);
        sqlite3_reset(ins);
    }
    sqlite3_finalize(ins);

    exec("COMMIT;");
}

// ─── queries ────────────────────────────────────────────────────────────────

std::vector<Country> Database::getCountries() const {
    std::vector<Country> result;
    sqlite3_stmt* s = nullptr;
    sqlite3_prepare_v2(m_db,
        "SELECT id, name, code, flag FROM countries ORDER BY name;",
        -1, &s, nullptr);
    while (sqlite3_step(s) == SQLITE_ROW) {
        Country c;
        c.id   = sqlite3_column_int(s, 0);
        c.name = col(s, 1);
        c.code = col(s, 2);
        c.flag = col(s, 3);
        result.push_back(c);
    }
    sqlite3_finalize(s);
    return result;
}

std::vector<Radio> Database::getRadiosByCountry(int countryId) const {
    std::vector<Radio> result;
    sqlite3_stmt* s = nullptr;
    sqlite3_prepare_v2(m_db,
        "SELECT id, name, stream_url, genre, description, logo_url, country_id "
        "FROM radios WHERE country_id = ? ORDER BY name;",
        -1, &s, nullptr);
    sqlite3_bind_int(s, 1, countryId);
    while (sqlite3_step(s) == SQLITE_ROW) {
        Radio r;
        r.id          = sqlite3_column_int(s, 0);
        r.name        = col(s, 1);
        r.streamUrl   = col(s, 2);
        r.genre       = col(s, 3);
        r.description = col(s, 4);
        r.logoUrl     = col(s, 5);
        r.countryId   = sqlite3_column_int(s, 6);
        result.push_back(r);
    }
    sqlite3_finalize(s);
    return result;
}

std::optional<Radio> Database::getRadioById(int radioId) const {
    sqlite3_stmt* s = nullptr;
    sqlite3_prepare_v2(m_db,
        "SELECT id, name, stream_url, genre, description, logo_url, country_id "
        "FROM radios WHERE id = ?;",
        -1, &s, nullptr);
    sqlite3_bind_int(s, 1, radioId);
    if (sqlite3_step(s) == SQLITE_ROW) {
        Radio r;
        r.id          = sqlite3_column_int(s, 0);
        r.name        = col(s, 1);
        r.streamUrl   = col(s, 2);
        r.genre       = col(s, 3);
        r.description = col(s, 4);
        r.logoUrl     = col(s, 5);
        r.countryId   = sqlite3_column_int(s, 6);
        sqlite3_finalize(s);
        return r;
    }
    sqlite3_finalize(s);
    return std::nullopt;
}
