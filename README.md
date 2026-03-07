# Radio App

A radio streaming application for **Tunisia**, **Belgium**, **United Kingdom**, and **France** — 35+ verified stations, built with C++20, Qt6/QML, and mpv.

---

## Running on Linux

There are two ways to run the app on Linux: **Docker** (easiest, no setup) or **native build** (best performance).

---

## Option 1 — Docker (recommended)

No need to install Qt6 or any build tools. Docker handles everything.

### Prerequisites

```bash
sudo apt install docker.io xhost
sudo systemctl enable --now docker
sudo usermod -aG docker $USER   # log out and back in after this
```

### Build and run

```bash
git clone <repo-url>
cd radio-app
bash docker-run.sh
```

The first run builds the Docker image (takes ~5 minutes). Every subsequent run starts in seconds.

The script automatically:
- Shares your X11 display so the window appears on your desktop
- Shares your PulseAudio socket so audio works
- Persists the station database across runs (Docker volume `radio-app-data`)

### Desktop shortcut (Ubuntu/GNOME)

1. Edit `radio-app.desktop` — replace `/path/to/radio-app/` with the actual path:
   ```ini
   Exec=/home/yourname/radio-app/docker-run.sh
   ```

2. Install it:
   ```bash
   cp radio-app.desktop ~/.local/share/applications/
   chmod +x radio-app.desktop
   ```

3. Search for "Radio App" in your app launcher, or right-click the desktop and pin it.

---

## Option 2 — Native build

### Prerequisites

Install all dependencies in one command (Ubuntu 24.04 / Debian 12):

```bash
sudo apt install -y \
    cmake ninja-build build-essential pkg-config \
    qt6-base-dev qt6-declarative-dev qt6-tools-dev qt6-tools-dev-tools \
    qml6-module-qtquick qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts qml6-module-qtquick-window \
    libqt6quickcontrols2-6 \
    libgl1-mesa-dev libgles2-mesa-dev \
    libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-randr0 \
    libxcb-render-util0 libxcb-shape0 libxcb-xinerama0 libxcb-xkb1 \
    libxkbcommon-x11-0 \
    libsqlite3-dev \
    mpv
```

### Build

```bash
git clone <repo-url>
cd radio-app
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build --target radio-app-gui
```

### Run

```bash
./build/radio-app-gui
```

Or use the launch script (auto-rebuilds if source changes):

```bash
bash start.sh
```

### Desktop shortcut (native)

1. Edit `radio-app.desktop`:
   ```ini
   Exec=/home/yourname/radio-app/build/radio-app-gui
   ```

2. Install it:
   ```bash
   cp radio-app.desktop ~/.local/share/applications/
   ```

---

## Troubleshooting

| Problem | Fix |
|---|---|
| No window appears (Docker) | Run `xhost +local:docker` before `docker-run.sh` |
| No audio (Docker) | Make sure PulseAudio is running: `pulseaudio --start` |
| `mpv: command not found` | Install mpv: `sudo apt install mpv` |
| Qt platform plugin error | Set `QT_QPA_PLATFORM=xcb` before running |
| Black screen / OpenGL error | Install `libgl1-mesa-dri` and try again |

---

## Architecture

```
radio-app/
├── src/
│   ├── database/       SQLite3 database (stations + countries)
│   ├── models/         Plain C++ data structs (Radio, Country)
│   ├── qml/            Qt bridge: RadioApp, CountryModel, RadioModel, PlayerController
│   ├── player/         CLI audio player (fork/exec mpv)
│   └── cli/            Interactive CLI interface
├── qml/                QML UI files (Main, CountrySidebar, RadioGrid, StationCard, NowPlayingBar)
├── Dockerfile          Ubuntu Docker image
├── docker-run.sh       Docker launcher (X11 + PulseAudio)
├── start.sh            macOS/Linux native launcher
└── make-app-bundle.sh  macOS .app bundle creator
```

Audio is played by **mpv** as a child process, controlled via its JSON IPC socket at `/tmp/radio-mpv-gui.sock`. Volume is adjusted in real time through the socket without restarting the stream.
