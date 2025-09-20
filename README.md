# 3D DICOM visualizer in C++

A DICOM viewer application built with Qt, VTK, and DCMTK.

## Features
- Load and view DICOM series
- Display associated contour data
- Adjustable transparency for slice viewing
- Time series navigation

## Sample Images (RV Contour)
<img width="1211" height="743" alt="image (2)" src="https://github.com/user-attachments/assets/db03c840-1454-4c87-8b2f-6cc1f2c47a56" />
<img width="1212" height="743" alt="image (3)" src="https://github.com/user-attachments/assets/ccaf50e9-e85b-4b10-ab51-ce12bfd0d7f1" />

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

