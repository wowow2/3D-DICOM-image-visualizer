#!/usr/bin/env bash
set -euo pipefail

if [[ "$(id -u)" -ne 0 ]]; then
    echo "Run with sudo: sudo ./scripts/install-vtk-apt.sh"
    exit 1
fi

TMPDIR="$(mktemp -d)"
trap 'rm -rf "${TMPDIR}"' EXIT
cd "${TMPDIR}"
apt-get download libvtk9-dev libvtk9-qt-dev >/dev/null
apt-get install -y libvtk9.1t64-qt
dpkg -i --force-depends libvtk9-dev*.deb libvtk9-qt-dev*.deb
apt-mark hold libvtk9-dev libvtk9-qt-dev
