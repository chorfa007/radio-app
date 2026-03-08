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
    exec("CREATE UNIQUE INDEX IF NOT EXISTS idx_radios_name ON radios(name);");
}

bool Database::isSeeded() const {
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(m_db, "PRAGMA user_version;", -1, &stmt, nullptr);
    sqlite3_step(stmt);
    int version = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return version >= 2;
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

        // ── Tunisia (extended from tunisie-radio.com) ─────────────────────────
        { "Shems FM",             "https://radio.shemsfm.net/shems",                        "Pop / Arabic",   "Pop and Arabic music",                  gf+"shemsfm.net.ico",         tn },
        { "IFM 100.6",            "https://live.ifm.tn/radio/8000/ifmlive?1585267848",       "Pop / Talk",     "IFM 100.6 — music & talk",              gf+"ifm.tn.ico",              tn },
        { "Zitouna FM",           "https://radio.radiotunisienne.tn/radiozaitouna",          "Religious",      "Islamic & Quranic broadcasting",        gf+"radiotunisienne.tn.ico",  tn },
        { "Radio Tunis Nationale","https://radio.radiotunisienne.tn/nationale",              "News / Culture", "Tunisia's national public radio",       gf+"radiotunisienne.tn.ico",  tn },
        { "KnOOz FM",             "http://streaming.knoozfm.net:8000/knoozfm",              "Pop / Hits",     "Hot hits & entertainment",              gf+"knoozfm.net.ico",         tn },
        { "Sabra FM",             "https://manager5.streamradio.fr:1905/stream",            "News / Talk",    "News and current affairs",              gf+"sabrafm.tn.ico",          tn },
        { "Cap FM",               "http://stream8.tanitweb.com/capfm",                      "Variety",        "Variety hits and entertainment",        gf+"tanitweb.com.ico",        tn },
        { "Radio Jeunes Tunisie", "https://radio.radiotunisienne.tn/jeunes",                "Youth / Pop",    "Tunisia's youth radio",                 gf+"radiotunisienne.tn.ico",  tn },
        { "Radio Monastir",       "https://radio.radiotunisienne.tn/monastir",              "Regional",       "Regional radio from Monastir",          gf+"radiotunisienne.tn.ico",  tn },
        { "RTCI",                 "https://radio.radiotunisienne.tn/rtci",                  "International",  "Radio Tunisienne Internationale",       gf+"radiotunisienne.tn.ico",  tn },
        { "Radio Panorama",       "https://stream.tun-radio.com/radio/8000/panorama_tunis", "Variety",        "Panorama radio from Tunis",             gf+"tanitweb.com.ico",        tn },
        { "Radio Quran",          "http://5.135.194.225:8000/live",                         "Religious",      "24/7 Quran recitation",                 gf+"radiocoranfm.com.ico",    tn },
        { "Radio Gafsa",          "https://radio.radiotunisienne.tn/gafsa",                 "Regional",       "Regional radio from Gafsa",             gf+"radiotunisienne.tn.ico",  tn },
        { "Radio Tataouine",      "https://radio.radiotunisienne.tn/tataouine",             "Regional",       "Regional radio from Tataouine",         gf+"radiotunisienne.tn.ico",  tn },
        { "Radio Misk FM",        "https://live.misk.art/stream",                           "Spiritual",      "Spiritual & Islamic music",             gf+"misk.art.ico",            tn },
        { "Radio Med Tunisie",    "http://stream6.tanitweb.com/radiomed",                   "Mediterranean",  "Mediterranean music & culture",         gf+"tanitweb.com.ico",        tn },
        { "Oasis FM Gabes",       "https://stream1.rcast.net/69919",                        "Regional",       "Regional radio from Gabes",             gf+"rcast.net.ico",           tn },
        { "Ulysse FM",            "http://51.178.31.38:8000/stream",                        "Regional",       "Radio from Djerba island",              gf+"tanitweb.com.ico",        tn },
        { "MFM Tunisie",          "https://mfmtunisie.toutech.net/mfmlive",                 "Pop / Hits",     "Top hits & music",                      gf+"toutech.net.ico",         tn },
        { "Radio Kelma FM",       "http://mdigital.agency:8000/radiokelma",                "Talk / News",    "Talk radio & news",                     gf+"tanitweb.com.ico",        tn },
        { "Ribat FM",             "http://streaming.ribatfm.tn:8000/live",                  "Regional",       "Regional FM from Monastir",             gf+"ribatfm.tn.ico",          tn },
        { "Nejma FM",             "http://188.166.109.186:8000/stream",                     "Pop / Arabic",   "Arabic pop hits",                       gf+"tanitweb.com.ico",        tn },
        { "Radio Kalima",         "http://radio.kalima-tunisie.info:8787/Live.m3u",         "News / Talk",    "Independent news and talk radio",       gf+"kalima-tunisie.info.ico", tn },
        { "Karama FM",            "https://hosting.studioradiomedia.fr:2685/stream",        "Regional",       "FM from Sidi Bouzid",                   gf+"tanitweb.com.ico",        tn },
        { "Oxygene FM",           "https://streaming.radiooxygene.tn/oxygenefm",            "Pop / Dance",    "Pop and dance from Bizerte",            gf+"radiooxygene.tn.ico",     tn },
        { "Radio 6",              "http://94.23.230.160:8000/stream.mp3",                   "Youth / Pop",    "Youth music radio from Tunis",          gf+"tanitweb.com.ico",        tn },
        { "Mouja FM",             "http://87.117.197.33:5258/;",                            "Pop / Arabic",   "Arabic pop and variety",                gf+"tanitweb.com.ico",        tn },
        { "Mosaique FM Tarab",    "https://radio.mosaiquefm.net/mosatarab",                 "Arabic Tarab",   "Classic Arabic tarab music",            gf+"mosaiquefm.net.ico",      tn },
        { "Dream FM",             "https://c23.radioboss.fm/stream/109",                    "Pop / Dance",    "Pop, dance and hits",                   gf+"tanitweb.com.ico",        tn },
        { "Saraha FM",            "http://ns326208.ip-37-59-9.eu:8000/sarahafm",            "Pop / Talk",     "Talk and entertainment",                gf+"tanitweb.com.ico",        tn },
        { "Mosaique FM DJ",       "https://radio.mosaiquefm.net/mosadj",                    "Electronic / DJ","DJ mixes and electronic music",         gf+"mosaiquefm.net.ico",      tn },
        { "Radio Orient",         "https://stream.rcs.revma.com/7hnrkawf4p8uv.mp3",         "Arabic",         "Arabic music and culture",              gf+"radioorient.com.ico",     tn },
        { "Radio Msaken",         "https://stream.rm-fm.net/live",                          "Regional",       "Regional radio from Msaken",            gf+"tanitweb.com.ico",        tn },
        { "Djerid FM",            "http://137.74.115.47:9998/;listen.mp3",                  "Regional",       "Regional FM from Tozeur-Djerid area",   gf+"tanitweb.com.ico",        tn },
        { "Banzart FM",           "http://87.117.208.118:16487/source.mp3",                 "Regional",       "FM radio from Bizerte (Banzart)",       gf+"tanitweb.com.ico",        tn },
        { "Radio Babnet",         "https://radio.babnet.net/?type=http",                    "News / Talk",    "Babnet — news and online radio",        gf+"babnet.net.ico",          tn },
        { "Twenssa FM",           "http://197.14.12.137:8000/live",                         "Tunisian Dialect","Tunisian music and culture",           gf+"tanitweb.com.ico",        tn },
        { "Salam FM",             "http://142.4.223.153:9992",                              "Religious",      "Islamic & religious broadcasting",      gf+"tanitweb.com.ico",        tn },
        { "Horria FM",            "http://horriafm.org:8000/listen.mp3.m3u",                "Pop / Arabic",   "Free radio — music and variety",        gf+"horriafm.org.ico",        tn },
        { "El Ayam FM",           "http://elayyem-radio.com:8020/live.mp3",                 "Regional",       "Regional FM from Tunisia",              gf+"elayyem-radio.com.ico",   tn },
        { "Data FM",              "http://91.134.137.140:8000/datafm",                      "Pop / Hits",     "Music and entertainment",               gf+"tanitweb.com.ico",        tn },
        { "Boukornine Radio",     "https://srv1.backendbroadcast.com:8010/direct",          "Regional",       "Radio from Boukornine area",            gf+"tanitweb.com.ico",        tn },
        { "Shems Gold",           "http://stream6.tanitweb.com/gold",                       "Arabic Oldies",  "Arabic classic hits",                   gf+"shemsfm.net.ico",         tn },
        { "One FM",               "http://95.154.254.81:30267/;",                           "Pop / Hits",     "One FM — hits and variety",             gf+"tanitweb.com.ico",        tn },
        { "Radio Tunisia Med",    "http://azuracast.conceptradio.fr/radio/8000/radio.mp3",  "Mediterranean",  "Mediterranean music and culture",       gf+"tanitweb.com.ico",        tn },
        { "Radio NEFZAWA",        "http://stream-58.zeno.fm/x26qrvf6by8uv?zs=SSJ17qnWTleWGt0uaPok2Q", "Regional", "Regional radio from Nefzawa",    gf+"zeno.fm.ico",             tn },
        { "IFM Rire & Chansons",  "https://live.ifm.tn/radio/8010/ifmweb1?1586535034",     "Comedy / Music", "IFM comedy and chanson channel",        gf+"ifm.tn.ico",              tn },
        { "Mosaique FM Espoir",   "https://webradio2.mosaiquefm.net/ESPOIR",                "Inspirational",  "Inspirational & spiritual music",       gf+"mosaiquefm.net.ico",      tn },
        { "Jawhara Hit",          "https://streaming2.toutech.net/jfmweb4",                 "Pop / Hits",     "Jawhara Hit web radio",                 gf+"jawharafm.net.ico",       tn },
        { "Jawhara Gold",         "https://streaming2.toutech.net/jfmweb2",                 "Arabic Oldies",  "Jawhara Gold — classic Arabic hits",    gf+"jawharafm.net.ico",       tn },
        { "Jawhara Tounsi",       "https://streaming2.toutech.net/jfmweb1",                 "Tunisian Pop",   "Tunisian local hits & pop",             gf+"jawharafm.net.ico",       tn },
        { "Jawhara Clubbing",     "https://streaming2.toutech.net/jfmweb3",                 "Electronic",     "Club music and DJ mixes",               gf+"jawharafm.net.ico",       tn },
        { "AlHayet FM",           "https://manager8.streamradio.fr:2885/stream",            "Religious",      "Islamic & cultural programming",        gf+"tanitweb.com.ico",        tn },

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
    exec("PRAGMA user_version = 2;");
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
