#!/usr/bin/env bash
#
# Builds a pinned VTK from source into third_party/vtk/install.
#
# Why: Ubuntu 24.04's `libvtk9-dev` package cannot be installed (it
# hard-depends on both `libgdal-dev` and `default-libmysqlclient-dev`, which
# conflict). Building VTK ourselves with only the modules used by DicomViewer
# is clean and reproducible. No root required (everything stays in the repo).
#
# Usage:
#   ./scripts/build-vtk.sh
#
# Requirements: cmake, a C++ compiler, Qt5 dev, X11/OpenGL dev headers
# (installed by scripts/setup-deps.sh).
set -euo pipefail

VTK_VERSION="${VTK_VERSION:-9.3.1}"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORKDIR="${REPO_ROOT}/third_party/vtk"
SRC_DIR="${WORKDIR}/src-${VTK_VERSION}"
BUILD_DIR="${WORKDIR}/build-${VTK_VERSION}"
INSTALL_DIR="${WORKDIR}/install"

command -v cmake >/dev/null 2>&1 || { echo "cmake is required. Run: sudo ./scripts/setup-deps.sh"; exit 1; }
command -v g++ >/dev/null 2>&1 || { echo "g++ is required. Run: sudo ./scripts/setup-deps.sh"; exit 1; }

VTK_CMAKE_DIR="${INSTALL_DIR}/lib/cmake/vtk-${VTK_VERSION}"
if [[ -f "${VTK_CMAKE_DIR}/VTKConfig.cmake" ]]; then
    echo "VTK ${VTK_VERSION} already installed at ${INSTALL_DIR}."
    exit 0
fi

echo "==> VTK ${VTK_VERSION}"
echo "    source : ${SRC_DIR}"
echo "    build  : ${BUILD_DIR}"
echo "    install: ${INSTALL_DIR}"
mkdir -p "${WORKDIR}"

# 1. Download the source tarball (GitHub mirror of the Kitware repository).
if [[ ! -d "${SRC_DIR}" ]]; then
    echo "==> Downloading VTK ${VTK_VERSION} source..."
    curl -L --fail -o "${WORKDIR}/vtk.tar.gz" \
        "https://github.com/Kitware/VTK/archive/refs/tags/v${VTK_VERSION}.tar.gz"
    tar -xzf "${WORKDIR}/vtk.tar.gz" -C "${WORKDIR}"
    rm -f "${WORKDIR}/vtk.tar.gz"
    # The archive extracts to VTK-<version>; normalize the directory name.
    if [[ -d "${WORKDIR}/VTK-${VTK_VERSION}" && ! -d "${SRC_DIR}" ]]; then
        mv "${WORKDIR}/VTK-${VTK_VERSION}" "${SRC_DIR}"
    fi
fi

# 2. Configure with only the module groups DicomViewer uses:
#    Rendering (RenderingCore/OpenGL2/InteractionStyle), Imaging (vtkImageFlip),
#    IO (vtkDICOMImageReader) and Qt (QVTKOpenGLNativeWidget).
#    Groups are set to WANT so that modules whose external dependencies are
#    missing are skipped rather than failing the build.
echo "==> Configuring VTK ${VTK_VERSION}..."
cmake -S "${SRC_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
    -DBUILD_SHARED_LIBS=ON \
    -DVTK_BUILD_TESTING=OFF \
    -DVTK_BUILD_EXAMPLES=OFF \
    -DVTK_BUILD_DOCUMENTATION=OFF \
    -DVTK_GROUP_ENABLE_Rendering=WANT \
    -DVTK_GROUP_ENABLE_Imaging=WANT \
    -DVTK_GROUP_ENABLE_IO=WANT \
    -DVTK_GROUP_ENABLE_Qt=WANT \
    -DVTK_GROUP_ENABLE_Filtering=DONT_WANT \
    -DVTK_GROUP_ENABLE_Views=DONT_WANT \
    -DVTK_GROUP_ENABLE_Web=DONT_WANT \
    -DVTK_GROUP_ENABLE_StandAlone=DONT_WANT \
    -DVTK_GROUP_ENABLE_Utilities=DONT_WANT

# 3. Build (parallel).
CORES="$(nproc)"
echo "==> Building VTK with ${CORES} cores (this takes ~10-15 minutes)..."
cmake --build "${BUILD_DIR}" -j"${CORES}"

# 4. Install into the local prefix.
echo "==> Installing VTK to ${INSTALL_DIR}..."
cmake --install "${BUILD_DIR}"

echo
echo "Done. VTK ${VTK_VERSION} is installed at:"
echo "  ${INSTALL_DIR}"
echo
echo "Build the application with:"
echo "  mkdir -p build && cd build"
echo "  cmake .."    # finds the local VTK automatically
echo "  cmake --build ."