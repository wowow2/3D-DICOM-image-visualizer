cmake_minimum_required(VERSION 3.16)
project(DicomViewer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Qt Setup ---
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

# --- VTK Setup ---
# If VTK was built locally by scripts/build-vtk.sh, point find_package at it.
# (Ubuntu 24.04's system libvtk9-dev is uninstallable due to a packaging bug.)
if(NOT VTK_DIR)
    file(GLOB _vtk_local_dirs
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/vtk/install/lib/cmake/vtk-*")
    if(_vtk_local_dirs)
        list(GET _vtk_local_dirs 0 VTK_DIR)
        message(STATUS "Using locally built VTK at: ${VTK_DIR}")
    endif()
endif()
# System VTK 9.1's config references many imported targets (MPI, Freetype,
# TBB, etc.) that are missing when the dev package was installed with
# --force-depends. Stub them so configuration succeeds; the app's 5 VTK
# components do not actually need most of these at link time.
foreach(_tgt IN ITEMS
    MPI::MPI_C MPI::MPI_CXX
    Freetype::Freetype TBB::tbb
    utf8cpp double-conversion::double
    LZ4::LZ4 JsonCpp::JsonCpp
    EXPAT::EXPAT GL2PS::GL2PS GLEW::GLEW
    JPEG::JPEG PNG::PNG TIFF::TIFF
    LibXml2::LibXml2 LZMA::LZMA ZLIB::ZLIB
    THEORA::THEORA THEORA::ENC THEORA::DEC
    OGG::OGG hdf5::hdf5 NetCDF::NetCDF
    LibPROJ::LibPROJ SQLite::SQLite3
    freetype
)
    if(NOT TARGET ${_tgt})
        add_library(${_tgt} INTERFACE IMPORTED)
    endif()
endforeach()
find_package(VTK REQUIRED COMPONENTS
    RenderingCore
    RenderingOpenGL2
    InteractionStyle
    IOImage
    GUISupportQt
)

# --- DCMTK Setup ---
# This command still finds the headers, which is good.
find_package(DCMTK REQUIRED)

# --- Find the Eigen3 math library ---
find_package(Eigen3 REQUIRED)

# --- Find cnpy library ---
include(FetchContent)
FetchContent_Declare(
    cnpy
    GIT_REPOSITORY https://github.com/rogersce/cnpy.git
    GIT_TAG master
)
FetchContent_MakeAvailable(cnpy)

# --- Define the Executable and ALL its sources (including headers) ---
add_executable(DicomViewer
    src/main.cpp
    src/MainWindow.cpp
    src/DicomManager.cpp
    src/VtkManager.cpp
    src/SeriesSelectionDialog.cpp
    src/ControlPanel.cpp
    include/MainWindow.h
    include/ControlPanel.h
    include/SeriesSelectionDialog.h
)

# --- Specify Include Directories ---
target_include_directories(DicomViewer PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    ${DCMTK_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIRS}
    ${cnpy_SOURCE_DIR}
)

# --- Link All Libraries ---
target_link_libraries(DicomViewer PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    
    VTK::GUISupportQt
    VTK::IOImage
    VTK::InteractionStyle
    VTK::RenderingOpenGL2

    dcmdata
    ofstd
    cnpy
)
