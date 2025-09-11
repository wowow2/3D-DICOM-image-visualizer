#pragma once

#include <QDialog> // Base class for dialog windows in Qt
#include <vector>
#include <string>

// Forward declarations
class QListWidget; // Qt widget for displaying a list of selectable items
class QDialogButtonBox; // Qt widget for standard dialog buttons (OK, Cancel, etc.)

// Dialog window for user to select DICOM series from a list. 
class SeriesSelectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit SeriesSelectionDialog(const std::vector<std::string>& seriesNames, QWidget *parent = nullptr);
    ~SeriesSelectionDialog();

    std::vector<std::string> getSelectedSeries() const; // Gives a vector of series selected by user

private:
    QListWidget* m_listWidget; // Widget displaying the list of series for selection
    QDialogButtonBox* m_buttonBox; // Container for standard dialog buttons (OK/Cancel)
};