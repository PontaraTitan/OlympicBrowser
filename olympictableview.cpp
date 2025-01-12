#include "olympictableview.h"
#include <QHeaderView>

OlympicTableView::OlympicTableView(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void OlympicTableView::setupUI() {
    mainLayout = new QVBoxLayout(this);

    // Create filter section
    QWidget* filterWidget = new QWidget(this);
    filtersLayout = new QVBoxLayout(filterWidget);

    QHBoxLayout* topControlsLayout = new QHBoxLayout;

    // Initialize buttons
    addFilterButton = new QPushButton("Add Filter", this);
    applyFiltersButton = new QPushButton("Apply Filters", this);
    clearAllButton = new QPushButton("Clear All", this);

    // Row count label
    rowCountLabel = new QLabel(this);
    rowCountLabel->setText("Displaying 0 out of 0 total rows");

    topControlsLayout->addWidget(addFilterButton);
    topControlsLayout->addWidget(applyFiltersButton);
    topControlsLayout->addWidget(clearAllButton);
    topControlsLayout->addStretch();
    topControlsLayout->addWidget(rowCountLabel);

    filtersLayout->addLayout(topControlsLayout);
    mainLayout->addWidget(filterWidget);

    // Create table view
    tableView = new QTableView(this);
    sourceModel = new OlympicTableModel(this);
    proxyModel = new OlympicFilterProxyModel(this);
    proxyModel->setSourceModel(sourceModel);
    tableView->setModel(proxyModel);

    // Configure table view
    tableView->setSortingEnabled(true);
    tableView->setAlternatingRowColors(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Adjust column widths after data is loaded
    QHeaderView* header = tableView->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Fixed);  // Prevent automatic resizing

    // Add extra space for sort indicator
    const int sortIndicatorWidth = 26;  // Typical width for sort indicator

    // Resize each column to content
    for(int column = 0; column < sourceModel->columnCount(); ++column) {
        tableView->resizeColumnToContents(column);
        // Add extra width for sort indicator
        int currentWidth = tableView->columnWidth(column);
        tableView->setColumnWidth(column, currentWidth + sortIndicatorWidth);
    }

    mainLayout->addWidget(tableView);

    // Connect signals
    connect(addFilterButton, &QPushButton::clicked, this, &OlympicTableView::addFilterRow);
    connect(applyFiltersButton, &QPushButton::clicked, this, &OlympicTableView::applyFilter);
    connect(clearAllButton, &QPushButton::clicked, this, &OlympicTableView::clearFilters);
    connect(proxyModel, &QSortFilterProxyModel::layoutChanged, this, &OlympicTableView::updateRowCountLabel);

    // Add initial filter row
    addFilterRow();

    updateRowCountLabel();
}

void OlympicTableView::createFilterRow() {
    FilterRow filterRow;

    filterRow.container = new QWidget(this);
    QHBoxLayout* rowLayout = new QHBoxLayout(filterRow.container);

    filterRow.columnCombo = new QComboBox(this);
    filterRow.columnCombo->addItems({"ID", "Name", "Sex", "Age", "Height", "Weight",
                                     "Team", "NOC", "Games", "Year", "Season", "City",
                                     "Sport", "Event", "Medal"});

    filterRow.patternEdit = new QLineEdit(this);
    filterRow.patternEdit->setPlaceholderText("Enter filter pattern (regex supported)");

    filterRow.removeButton = new QPushButton("Remove", this);

    rowLayout->addWidget(filterRow.columnCombo);
    rowLayout->addWidget(filterRow.patternEdit);
    rowLayout->addWidget(filterRow.removeButton);

    filtersLayout->addWidget(filterRow.container);

    connect(filterRow.removeButton, &QPushButton::clicked, this, &OlympicTableView::removeFilterRow);
    connect(filterRow.patternEdit, &QLineEdit::returnPressed, this, &OlympicTableView::applyFilter);

    filterRows.append(filterRow);
}

void OlympicTableView::addFilterRow() {
    createFilterRow();
}

void OlympicTableView::removeFilterRow() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    for (int i = 0; i < filterRows.size(); ++i) {
        if (filterRows[i].removeButton == button) {
            filterRows[i].container->deleteLater();
            filterRows.remove(i);
            applyFilter();
            break;
        }
    }
}

void OlympicTableView::updateRowCountLabel() {
    int visibleRows = proxyModel->rowCount();
    int totalRows = sourceModel->rowCount();
    rowCountLabel->setText(QString("Displaying %1 out of %2 total rows")
                               .arg(visibleRows)
                               .arg(totalRows));
}

void OlympicTableView::applyFilter() {
    proxyModel->clearFilters();

    for (const FilterRow& row : filterRows) {
        QString pattern = row.patternEdit->text();
        if (!pattern.isEmpty()) {
            proxyModel->setColumnFilter(row.columnCombo->currentIndex(), pattern);
        }
    }
    updateRowCountLabel();
}

void OlympicTableView::clearFilters() {
    for (const FilterRow& row : filterRows) {
        row.patternEdit->clear();
    }
    proxyModel->clearFilters();
    updateRowCountLabel();
}
