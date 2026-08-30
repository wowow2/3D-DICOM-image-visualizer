#!/usr/bin/env bash
set -euo pipefail

PKGS=(
    cmake
    build-essential
    qtbase5-dev
    libdcmtk-dev
    libeigen3-dev
    libgl-dev
    libgl1-mesa-dev
    libglu1-mesa-dev
    libx11-dev
    libxext-dev
    libxt-dev
    xdotool
)

if [[ "$(id -u)" -ne 0 ]]; then
    echo "Run with sudo: sudo ./scripts/setup-deps.sh"
    exit 1
fi

apt-get update
apt-get install -y "${PKGS[@]}"

for pkg in "${PKGS[@]}"; do
    dpkg -s "$pkg" >/dev/null 2>&1 && echo "[OK] $pkg" || echo "[WARN] $pkg"
done
