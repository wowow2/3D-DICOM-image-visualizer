#!/usr/bin/env bash
set -euo pipefail

VTK_VERSION="${VTK_VERSION:-9.3.1}"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORKDIR="${REPO_ROOT}/third_party/vtk"
SRC_DIR="${WORKDIR}/src-${VTK_VERSION}"
BUILD_DIR="${WORKDIR}/build-${VTK_VERSION}"
INSTALL_DIR="${WORKDIR}/install"

command -v cmake >/dev/null 2>&1 || { echo "cmake not found"; exit 1; }
command -v g++ >/dev/null 2>&1 || { echo "g++ not found"; exit 1; }

if [[ -f "${INSTALL_DIR}/lib/cmake/vtk-${VTK_VERSION}/vtk-config.cmake" ]]; then
    echo "VTK ${VTK_VERSION} already installed at ${INSTALL_DIR}"
    exit 0
fi

mkdir -p "${WORKDIR}"

if [[ ! -d "${SRC_DIR}" ]]; then
    curl -L --fail -o "${WORKDIR}/vtk.tar.gz" \
        "https://github.com/Kitware/VTK/archive/refs/tags/v${VTK_VERSION}.tar.gz"
    tar -xzf "${WORKDIR}/vtk.tar.gz" -C "${WORKDIR}"
    rm -f "${WORKDIR}/vtk.tar.gz"
    [[ -d "${WORKDIR}/VTK-${VTK_VERSION}" ]] && mv "${WORKDIR}/VTK-${VTK_VERSION}" "${SRC_DIR}"
fi

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
    -DVTK_GROUP_ENABLE_Utilities=DONT_WANT \
    -DVTK_MODULE_ENABLE_VTK_GUISupportQtQuick=DONT_WANT \
    -DVTK_MODULE_ENABLE_VTK_GUISupportQtSQL=DONT_WANT \
    -DVTK_MODULE_ENABLE_VTK_RenderingQt=DONT_WANT \
    -DVTK_MODULE_ENABLE_VTK_ViewsQt=DONT_WANT

cmake --build "${BUILD_DIR}" -j"$(nproc)"
cmake --install "${BUILD_DIR}"
