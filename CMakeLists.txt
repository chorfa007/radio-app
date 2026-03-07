cmake_minimum_required(VERSION 3.20)
project(RadioApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# ── compiler warnings ────────────────────────────────────────────────────────
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow)
endif()

# ── SQLite3 ──────────────────────────────────────────────────────────────────
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

# ── shared core library (Database, models) ────────────────────────────────────
add_library(radio-core STATIC
    src/database/Database.cpp
)
target_include_directories(radio-core PUBLIC
    src
    ${SQLITE3_INCLUDE_DIRS}
)
target_link_libraries(radio-core PUBLIC ${SQLITE3_LIBRARIES})
target_compile_options(radio-core PUBLIC ${SQLITE3_CFLAGS_OTHER})

# ── CLI target ────────────────────────────────────────────────────────────────
add_executable(radio-app
    src/main.cpp
    src/player/Player.cpp
    src/cli/CLI.cpp
)
target_link_libraries(radio-app PRIVATE radio-core)

# ── Qt6 GUI target ────────────────────────────────────────────────────────────
find_package(Qt6 QUIET COMPONENTS Core Qml Quick QuickControls2 Network)

if(Qt6_FOUND)
    message(STATUS "Qt6 found: ${Qt6_VERSION} — building radio-app-gui")

    qt_standard_project_setup(REQUIRES 6.5)

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)

    qt_add_executable(radio-app-gui
        src/gui_main.cpp
        src/qml/CountryModel.cpp
        src/qml/RadioModel.cpp
        src/qml/PlayerController.cpp
        src/qml/RadioApp.cpp
    )

    qt_add_qml_module(radio-app-gui
        URI     RadioApp
        VERSION 1.0
        QML_FILES
            qml/Main.qml
            qml/CountrySidebar.qml
            qml/RadioGrid.qml
            qml/StationCard.qml
            qml/NowPlayingBar.qml
    )

    target_include_directories(radio-app-gui PRIVATE src)

    target_link_libraries(radio-app-gui PRIVATE
        radio-core
        Qt6::Core
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Network
    )

    install(TARGETS radio-app-gui DESTINATION bin)
else()
    message(WARNING "Qt6 not found — skipping radio-app-gui target")
endif()

# ── install CLI ───────────────────────────────────────────────────────────────
install(TARGETS radio-app DESTINATION bin)

message(STATUS "C++ standard : C++${CMAKE_CXX_STANDARD}")
message(STATUS "SQLite3      : ${SQLITE3_VERSION}")
message(STATUS "Build type   : ${CMAKE_BUILD_TYPE}")
