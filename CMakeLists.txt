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
