# DICOMVIS

A DICOM viewer application built with Qt, VTK, and DCMTK.

## Features
- Load and view DICOM series
- Display associated contour data
- Adjustable transparency for slice viewing
- Time series navigation

## Building
Requires:
- Qt5
- VTK
- DCMTK
- Eigen3

```bash
mkdir build && cd build
cmake ..
cmake --build .
```
## Running
After the build process is complete, the executable will be located inside the build directory. To start the DICOMVIS application, run the following command from within the build directory:

```bash
./DICOMVIS
```

