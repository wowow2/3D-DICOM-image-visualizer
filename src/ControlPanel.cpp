#include "ControlPanel.h"

#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QCheckBox> 
#include <QHBoxLayout>

// Constructs the control panel with all UI components
ControlPanel::ControlPanel(QWidget *parent) : QWidget(parent) {
    // Create Widgets
    m_loadPatientButton = new QPushButton("Load Patient");
    m_frameSlider = new QSlider(Qt::Horizontal);
    m_frameLabel = new QLabel("--/--");
    m_transparencyToggle = new QCheckBox("Transparent Slices"); 

    // Set Initial State
    setControlsEnabled(false);
    m_transparencyToggle->setChecked(true); // Start with transparency on

    // Layout
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_loadPatientButton); // Load patient button
    layout->addWidget(m_frameSlider, 1); // Frame slider 
    layout->addWidget(m_frameLabel); // Frame information label
    layout->addWidget(m_transparencyToggle); // Transparency toggle checkbox

    // Connect signals to slots
    connect(m_loadPatientButton, &QPushButton::clicked, this, &ControlPanel::loadPatientClicked); // Handle load patient button
    connect(m_transparencyToggle, &QCheckBox::toggled, this, &ControlPanel::transparencyToggled);  // Transparency toggle changes
}

ControlPanel::~ControlPanel() {}

// Set frame range
void ControlPanel::setFrameSliderRange(int min, int max) {
    m_frameSlider->setRange(min, max);
}

// Set frame navigation controls
void ControlPanel::setControlsEnabled(bool enabled) {
    m_frameSlider->setEnabled(enabled);
    m_frameLabel->setEnabled(enabled);
}

// Updates the frame label to show current position and total frames
void ControlPanel::updateFrameLabel(int currentFrame, int maxFrame) {
    m_frameLabel->setText(QString("%1 / %2").arg(currentFrame + 1).arg(maxFrame + 1));
}

// Provides access to the frame slider widget
QSlider* ControlPanel::getFrameSlider() const {
    return m_frameSlider;
}