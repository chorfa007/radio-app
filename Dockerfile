# Radio App — Ubuntu Docker image
# Supports X11 display forwarding + PulseAudio for audio output
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV QT_QPA_PLATFORM=xcb

# ── System deps ───────────────────────────────────────────────────────────────
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Build tools
    cmake ninja-build build-essential pkg-config git \
    # Qt6 + QML + Controls2 Material
    qt6-base-dev \
    qt6-declarative-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    qml6-module-qtquick \
    qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts \
    qml6-module-qtquick-window \
    libqt6quickcontrols2-6 \
    qml6-module-qt-labs-qmlmodels \
    # OpenGL (needed by Qt Quick)
    libgl1-mesa-dev \
    libgles2-mesa-dev \
    libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-randr0 \
    libxcb-render-util0 libxcb-shape0 libxcb-xinerama0 libxcb-xkb1 \
    libxkbcommon-x11-0 \
    # SQLite
    libsqlite3-dev \
    # Audio player
    mpv \
    # PulseAudio client (for audio passthrough)
    libpulse0 pulseaudio-utils \
    && rm -rf /var/lib/apt/lists/*

# ── Copy source & build ───────────────────────────────────────────────────────
WORKDIR /app
COPY CMakeLists.txt .
COPY src/ src/
COPY qml/ qml/

RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -G Ninja \
    && cmake --build build --target radio-app-gui

# ── Runtime ───────────────────────────────────────────────────────────────────
CMD ["/app/build/radio-app-gui"]
