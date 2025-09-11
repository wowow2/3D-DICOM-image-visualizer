#pragma once

#include <QMainWindow> // Base class for main window
#include "DicomManager.h"
#include "VtkManager.h"

// Forward declarations
class QVTKOpenGLNativeWidget; // QT widget that embeds VTK rendering
class ControlPanel; // Custom control panel UI component

/**
 * The main application window class that coordinates all components.
 * This class serves as the central hub connecting the UI, data management,
 * and visualization components of the DICOM viewer.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Slots are methods that respond to specific signals (events)
    void onLoadPatient(); // Handles the "Load Patient" button click event
    void onSliderMoved(int frameIndex);  // Responds to frame slider movement
    void onSliderReleased(); // Responds to frame slider release
    void onTransparencyToggled(bool isTransparent); // Handles transparency toggle checkbox state changes

private:
    void setupConnections(); // Establishes communication between UI components and application logic

    // UI Components
    QVTKOpenGLNativeWidget* m_vtkWidget; // Widget that hosts VTK visualization
    ControlPanel* m_controlPanel; // Custom panel with controls (sliders, buttons, et

    // Core Logic and Data Components
    DicomManager m_dicomManager; // Handles DICOM file loading and management
    VtkManager m_vtkManager; // Manages VTK visualization pipeline and rendering
};