#include "MainWindow.h"
#include "ControlPanel.h"
#include "SeriesSelectionDialog.h"

#include <QVTKOpenGLNativeWidget.h>
#include <QVBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QSlider>
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QStatusBar>
#include <vtkRenderWindow.h>

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
    QSettings settings;
    QString lastDir = settings.value("lastPatientDir", QDir::homePath()).toString();

    QString patientPath = QFileDialog::getExistingDirectory(this, "Select Patient Directory", lastDir);
    if (patientPath.isEmpty()) {
        return;
    }

    std::vector<std::string> seriesNames = m_dicomManager.discoverSeries(patientPath.toStdString());

    if (seriesNames.empty()) {
        QMessageBox::warning(this, "No series found",
                             "No series sub-directories found in the selected path.");
        statusBar()->showMessage("No series found", 5000);
        return;
    }

    SeriesSelectionDialog dialog(seriesNames, this);
    if (dialog.exec() == QDialog::Accepted) {
        std::vector<std::string> selectedSeries = dialog.getSelectedSeries();
        if (selectedSeries.empty()) {
            statusBar()->showMessage("No series selected", 5000);
            return;
        }

        statusBar()->showMessage(QString("Loading %1 series...").arg(selectedSeries.size()));

        if (m_dicomManager.loadSelectedSeries(patientPath.toStdString(), selectedSeries)) {
            settings.setValue("lastPatientDir", patientPath);

            int numFrames = m_dicomManager.getNumberOfFrames();

            if (numFrames > 1) {
                m_controlPanel->setFrameSliderRange(0, numFrames - 1);
                m_controlPanel->setControlsEnabled(true);
                m_controlPanel->getFrameSlider()->setValue(0);
            } else {
                m_controlPanel->setControlsEnabled(false);
                if (numFrames == 1) {
                    m_controlPanel->setFrameSliderRange(0,0);
                    m_controlPanel->updateFrameLabel(0,0);
                }
            }

            QString patientName = QFileInfo(patientPath).fileName();
            setWindowTitle(QString("Dicom Viewer — %1 — %2 series · %3 frames")
                               .arg(patientName)
                               .arg(selectedSeries.size())
                               .arg(numFrames));
            statusBar()->showMessage(QString("Loaded %1 — %2 series · %3 frames")
                                         .arg(patientName)
                                         .arg(selectedSeries.size())
                                         .arg(numFrames),
                                     8000);

            onSliderReleased();
            m_vtkManager.resetCamera();

        } else {
            QMessageBox::critical(this, "Load failed",
                                  "Failed to load DICOM data from the selected series.");
            statusBar()->showMessage("Load failed", 5000);
        }
    } else {
        statusBar()->showMessage("Series selection canceled", 4000);
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
    statusBar()->showMessage(QString("Frame %1").arg(frameIndex + 1), 3000);

    std::vector<DicomFrame> frames = m_dicomManager.getFramesForTimepoint(frameIndex);
    m_vtkManager.createScene(frames);

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