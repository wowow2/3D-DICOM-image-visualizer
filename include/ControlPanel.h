#pragma once

#include <QWidget> // Base class for widgets in Qt

// Forward declarations
class QPushButton;
class QSlider;
class QLabel;
class QCheckBox;

class ControlPanel : public QWidget {
    Q_OBJECT // Qt macro required for any class that uses signals/slots

public:
    explicit ControlPanel(QWidget *parent = nullptr);
    ~ControlPanel();

    // Public methods (unchanged)
    void setFrameSliderRange(int min, int max);  // Sets the minimum and maximum values for the frame slider
    void setControlsEnabled(bool enabled); // Enables/disables all controls in the panel
    void updateFrameLabel(int currentFrame, int maxFrame); // Updates the frame label text
    QSlider* getFrameSlider() const; // Getter method for the frame slider widget 

signals:
    void loadPatientClicked(); // Signal emitted when the load patient button is clicked
    void transparencyToggled(bool isTransparent); // Signal emitted when transparency toggle checkbox changes state

private:
    QPushButton* m_loadPatientButton; // Button to trigger patient data loading
    QSlider* m_frameSlider; // Slider for navigating through frames
    QLabel* m_frameLabel; // Displays current frame information
    QCheckBox* m_transparencyToggle; // Checkbox to toggle transparency mode
};