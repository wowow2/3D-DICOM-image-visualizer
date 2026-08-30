#!/usr/bin/env bash
#
# Installs the system build dependencies for DicomViewer on Debian/Ubuntu.
#
# Usage:
#   sudo ./scripts/setup-deps.sh
#
# Installs: CMake, a C++17 toolchain, Qt5 (dev), DCMTK, Eigen3, and the
# X11/OpenGL dev headers needed to build VTK from source.
#
# NOTE on VTK: Ubuntu 24.04's system package `libvtk9-dev` is uninstallable
# with a normal apt install (an upstream packaging bug: it hard-depends on
# both `libgdal-dev` and `default-libmysqlclient-dev`, which conflict). The
# provided workaround script `install-vtk-apt.sh` installs the official VTK
# 9.1 headers via dpkg; alternatively `build-vtk.sh` builds a pinned VTK from
# source. On other distros, install VTK with your normal package manager.
set -euo pipefail

PKGS=(
    cmake
    build-essential
    qtbase5-dev
    libdcmtk-dev
    libeigen3-dev
    # X11 / OpenGL dev headers needed by VTK's rendering + Qt support:
    libgl-dev
    libgl1-mesa-dev
    libglu1-mesa-dev
    libx11-dev
    libxext-dev
    libxt-dev
)

if [[ "$(id -u)" -ne 0 ]]; then
    echo "This script needs root privileges to install packages."
    echo "Re-run it with sudo, e.g.:  sudo ./scripts/setup-deps.sh"
    exit 1
fi

echo "==> Updating package lists..."
apt-get update

echo "==> Installing build dependencies..."
apt-get install -y "${PKGS[@]}"

echo
echo "==> Verifying installation..."
for pkg in "${PKGS[@]}"; do
    if dpkg -s "$pkg" >/dev/null 2>&1; then
        echo "  [OK]   $pkg"
    else
        echo "  [WARN] $pkg not found after install"
    fi
done

echo
echo "Done. Next steps:"
echo
echo "  1) Install VTK (see the note above; on Ubuntu 24.04 this needs the"
echo "     workaround script, on other distros a normal package install):"
echo "       sudo ./scripts/install-vtk-apt.sh   # Ubuntu 24.04 workaround"
echo "       # or build a pinned VTK from source (no root, ~15 min):"
echo "       ./scripts/build-vtk.sh"
echo
echo "  2) Build the application:"
echo "       mkdir -p build && cd build"
echo "       cmake ..   # picks up the local VTK automatically"
echo "       cmake --build ."