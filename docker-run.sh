#!/bin/bash
# Run Radio App in Docker on Ubuntu (with X11 + PulseAudio)
set -e

IMAGE="radio-app:latest"
CONTAINER="radio-app-gui"

# ── Build image if needed ─────────────────────────────────────────────────────
if ! docker image inspect "$IMAGE" &>/dev/null; then
    echo "Building Docker image (first run — takes a few minutes)..."
    docker build -t "$IMAGE" "$(dirname "$0")"
fi

# ── Stop any previous container ───────────────────────────────────────────────
docker rm -f "$CONTAINER" 2>/dev/null || true

# ── Allow local X11 connections ───────────────────────────────────────────────
xhost +local:docker 2>/dev/null || true

# ── Detect PulseAudio socket ─────────────────────────────────────────────────
PULSE_SOCKET="/run/user/$(id -u)/pulse/native"
PULSE_ARGS=""
if [ -S "$PULSE_SOCKET" ]; then
    PULSE_ARGS="-v $PULSE_SOCKET:/run/pulse/native -e PULSE_SERVER=unix:/run/pulse/native"
fi

# ── Launch ───────────────────────────────────────────────────────────────────
docker run --rm \
    --name "$CONTAINER" \
    -e DISPLAY="$DISPLAY" \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    $PULSE_ARGS \
    -v radio-app-data:/root/.local/share/radio-app \
    "$IMAGE"
