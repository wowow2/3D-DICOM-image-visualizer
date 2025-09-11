#include "SeriesSelectionDialog.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QDialogButtonBox>
#include <QVBoxLayout>

// Constructs a series selection dialog
SeriesSelectionDialog::SeriesSelectionDialog(const std::vector<std::string>& seriesNames, QWidget *parent)
    : QDialog(parent) // Initialize base QDialog class
{
    // Set dialog window properties
    setWindowTitle("Select Series to Load");
    setMinimumSize(400, 300);

    
    // Create UI widgets
    m_listWidget = new QListWidget();
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Populate the list with series names
    for (const auto& name : seriesNames) {
        // Create a list item for each series
        auto* item = new QListWidgetItem(QString::fromStdString(name), m_listWidget);
        
        // Make the item checkable (user can select/deselect it)
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        
        // Check it by default for convenience
        item->setCheckState(Qt::Checked);
    }

    // Set up the dialog layout
    QVBoxLayout *layout = new QVBoxLayout(this); // Vertical box layout
    layout->addWidget(m_listWidget); // Add list widget
    layout->addWidget(m_buttonBox); // Add button box at the bottom

    // Connect button signals to dialog slots
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SeriesSelectionDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SeriesSelectionDialog::reject);
}

SeriesSelectionDialog::~SeriesSelectionDialog() {}

// Retrieves the names of all selected series
std::vector<std::string> SeriesSelectionDialog::getSelectedSeries() const {
    std::vector<std::string> selected;
    // Iterate through all items in the list widget
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        // Add to selection if the item is checked
        if (item->checkState() == Qt::Checked) {
            selected.push_back(item->text().toStdString());
        }
    }
    return selected;
}