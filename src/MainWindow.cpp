#include "MainWindow.h"
#include "ControlPanel.h"
#include "SeriesSelectionDialog.h"

#include <QVTKOpenGLNativeWidget.h>
#include <QVBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QSlider>
#include <vtkRenderWindow.h>
#include <iostream>

// Construct main application
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Set window properties
    setWindowTitle("Dicom Viewer");
    resize(1280, 760);

    // Create central widget and main layout
    QWidget *centralWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout();
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // Create and add the visualization and control components
    m_vtkWidget = new QVTKOpenGLNativeWidget();
    m_controlPanel = new ControlPanel();

    // Add widgets to layout 
    mainLayout->addWidget(m_vtkWidget, 1);
    mainLayout->addWidget(m_controlPanel);

    // Initialize VTK manager and connect signals to slots
    m_vtkManager.setup(m_vtkWidget);
    setupConnections();
}

// Destructor  
MainWindow::~MainWindow() {}

// Establishes all signal-slot connections between UI and application logic
void MainWindow::setupConnections() {
    // Connect control panel signals to corresponding slots
    connect(m_controlPanel, &ControlPanel::loadPatientClicked, this, &MainWindow::onLoadPatient);
    // Get frame slider from control panel and connect its signals
    QSlider* slider = m_controlPanel->getFrameSlider();
    connect(slider, &QSlider::valueChanged, this, &MainWindow::onSliderMoved);
    connect(slider, &QSlider::sliderReleased, this, &MainWindow::onSliderReleased);
    // Connect transparency toggle signal
    connect(m_controlPanel, &ControlPanel::transparencyToggled, this, &MainWindow::onTransparencyToggled);

}

// Handles the "Load Patient" button click event
void MainWindow::onLoadPatient() {
    // Open directory selection dialog
    QString patientPath = QFileDialog::getExistingDirectory(this, "Select Patient Directory",
                                                       "/mnt/c/Users/abida/OneDrive/Desktop/Compute_Volume/Dataset/sa_dicom");
    if (patientPath.isEmpty()) { 
        // User cancelled the dialog
        return;
    }

    // Discover available DICOM series in the selected directory
    std::vector<std::string> seriesNames = m_dicomManager.discoverSeries(patientPath.toStdString());

    if (seriesNames.empty()) {
        std::cout << "No series sub-directories found in the selected path." << std::endl;
        return;
    }

    // Show series selection dialog to the user
    SeriesSelectionDialog dialog(seriesNames, this);
    if (dialog.exec() == QDialog::Accepted) {
        std::vector<std::string> selectedSeries = dialog.getSelectedSeries();
        if (selectedSeries.empty()) {
            std::cout << "User did not select any series." << std::endl;
            return;
        }

        std::cout << "--- Loading " << selectedSeries.size() << " selected series... ---" << std::endl;
        
        // Load the selected DICOM series
        if (m_dicomManager.loadSelectedSeries(patientPath.toStdString(), selectedSeries)) {
            // Get the number of frames in the longest series
            int numFrames = m_dicomManager.getNumberOfFrames();

            if (numFrames > 1) {
                // Enable controls and set slider range, assuming multiple frames
                m_controlPanel->setFrameSliderRange(0, numFrames - 1);
                m_controlPanel->setControlsEnabled(true);
                m_controlPanel->getFrameSlider()->setValue(0);
            } else {
                // There is only one frame
                m_controlPanel->setControlsEnabled(false);
                if (numFrames == 1) {
                    m_controlPanel->setFrameSliderRange(0,0);
                    m_controlPanel->updateFrameLabel(0,0);
                }
            }
            
            // Update the visualization and reset camera view
            onSliderReleased(); // Load and display the first frame
            m_vtkManager.resetCamera(); // Adjust camera to fit all objects

        } else {
            std::cout << "Failed to load DICOM data from the selected series." << std::endl;
        }
    } else {
        std::cout << "User canceled series selection." << std::endl;
    }
}

// Handles frame slider movement events
void MainWindow::onSliderMoved(int frameIndex) {
    int numFrames = m_dicomManager.getNumberOfFrames();
    // Update label (convert from 0 based index to 1 for display)
    m_controlPanel->updateFrameLabel(frameIndex, numFrames > 0 ? numFrames - 1 : 0);
}

// Handles frame slider release events
void MainWindow::onSliderReleased() {
    int frameIndex = m_controlPanel->getFrameSlider()->value();
    std::cout << "Updating scene to frame " << frameIndex << std::endl;

    // Get all frames for the selected timepoint and update visualization
    std::vector<DicomFrame> frames = m_dicomManager.getFramesForTimepoint(frameIndex);
    m_vtkManager.createScene(frames);

    // Trigger rendering of the updated scene
    m_vtkWidget->renderWindow()->Render();
}

// Handles transparency toggle events
void MainWindow::onTransparencyToggled(bool isTransparent) {
    if (isTransparent) {
        // The user wants transparency on
        m_vtkManager.setSliceOpacity(0.7); // Set to our default transparent value
    } else {
        // The user wants transparency off
        m_vtkManager.setSliceOpacity(1.0); // Set to fully opaque
    }
}