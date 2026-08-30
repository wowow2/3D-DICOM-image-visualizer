#!/usr/bin/env bash
#
# Installs the official Ubuntu VTK 9.1 development packages via a dpkg
# workaround (fast path).
#
# Why this exists: on Ubuntu 24.04 (noble), `libvtk9-dev` cannot be installed
# with a normal `apt-get install` due to an upstream packaging bug — it
# hard-depends on both `libgdal-dev` (which needs `libmariadb-dev-compat`)
# and `default-libmysqlclient-dev` (which needs MySQL's `libmysqlclient-dev`),
# and those two conflict with each other. This is a documented, unfixed
# Ubuntu bug; Debian and other distros install VTK normally.
#
# Workaround: the VTK *runtime* libraries install fine, so we:
#   1. install the Qt runtime module cleanly via apt (libvtk9.1t64-qt),
#   2. install only the two *header* packages with dpkg, skipping the broken
#      dependency checks (--force-depends),
#   3. hold those two packages so apt never tries to "repair"/remove them.
#
# Result: proper headers in /usr/include/vtk-9.1, working CMake config, and a
# normal `find_package(VTK)` for the application build. The only side effect
# is that apt will report those two packages as having unmet dependencies
# (cosmetic; they are held).
#
# Usage:
#   sudo ./scripts/install-vtk-apt.sh
#
# An alternative, 100%-pristine route (no root, ~15 min) is building a pinned
# VTK from source:  ./scripts/build-vtk.sh
set -euo pipefail

if [[ "$(id -u)" -ne 0 ]]; then
    echo "This script needs root privileges."
    echo "Re-run it with sudo, e.g.:  sudo ./scripts/install-vtk-apt.sh"
    exit 1
fi

echo "==> Downloading VTK 9.1 packages..."
TMPDIR_="$(mktemp -d)"
trap 'rm -rf "${TMPDIR_}"' EXIT
cd "${TMPDIR_}"
apt-get download libvtk9-dev libvtk9-qt-dev >/dev/null

echo "==> Installing the Qt runtime module (clean, via apt)..."
apt-get install -y libvtk9.1t64-qt

echo "==> Installing the VTK header packages (dpkg, skipping broken deps)..."
dpkg -i --force-depends libvtk9-dev*.deb libvtk9-qt-dev*.deb

echo "==> Holding header packages so apt leaves them alone..."
apt-mark hold libvtk9-dev libvtk9-qt-dev

echo
echo "==> Verifying..."
check() {
    if [[ -e "$1" ]]; then echo "  [OK]   $1"; else echo "  [WARN] missing: $1"; fi
}
check /usr/include/vtk-9.1/vtkRenderer.h
check /usr/include/vtk-9.1/QVTKOpenGLNativeWidget.h
check /usr/lib/x86_64-linux-gnu/cmake/vtk-9.1/VTKConfig.cmake
check /usr/lib/x86_64-linux-gnu/libvtkGUISupportQt-9.1.so.1
check /usr/lib/x86_64-linux-gnu/libvtk9.1-qt.so.1

echo
echo "Done. Build the application with:"
echo "  mkdir -p build && cd build"
echo "  cmake .."
echo "  cmake --build ."