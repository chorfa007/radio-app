#include "database/Database.h"
#include "qml/RadioApp.h"
#include "qml/CountryModel.h"
#include "qml/RadioModel.h"
#include "qml/PlayerController.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <filesystem>
#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("Radio App");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("RadioApp");

    QQuickStyle::setStyle("Material");

    // ── database ──────────────────────────────────────────────────────────
    const char* home = std::getenv("HOME");
    std::string dbDir = home ? std::string(home) + "/.local/share/radio-app" : ".";
    std::filesystem::create_directories(dbDir);

    Database db(dbDir + "/radios.db");
    db.initSchema();
    if (!db.isSeeded()) db.seedData();

    RadioApp radioApp(db);

    // ── QML engine ────────────────────────────────────────────────────────
    qmlRegisterUncreatableType<CountryModel>   ("RadioApp", 1, 0, "CountryModel",    "Use radioApp.countries");
    qmlRegisterUncreatableType<RadioModel>     ("RadioApp", 1, 0, "RadioModel",      "Use radioApp.radios");
    qmlRegisterUncreatableType<PlayerController>("RadioApp", 1, 0, "PlayerController","Use radioApp.player");

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("radioApp", &radioApp);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);

    engine.loadFromModule("RadioApp", "Main");

    return app.exec();
}
