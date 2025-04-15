#include <QFileDialog>
#include <QStandardPaths>
#include <QHeaderView>

#include "olympictableview.h"
#include "exportmanager.h"

OlympicTableView::OlympicTableView(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void OlympicTableView::setupUI() {
    mainLayout = new QVBoxLayout(this);

    QWidget* filterWidget = new QWidget(this);
    filtersLayout = new QVBoxLayout(filterWidget);

    QHBoxLayout* topControlsLayout = new QHBoxLayout;

    addFilterButton = new QPushButton("Add Filter", this);
    applyFiltersButton = new QPushButton("Apply Filters", this);
    clearAllButton = new QPushButton("Clear All", this);
    exportButton = new QPushButton("Export Data", this);
    reportButton = new QPushButton("Generate Report", this);

    rowCountLabel = new QLabel(this);
    rowCountLabel->setText("Displaying 0 out of 0 total rows");

    topControlsLayout->addWidget(addFilterButton);
    topControlsLayout->addWidget(applyFiltersButton);
    topControlsLayout->addWidget(clearAllButton);
    topControlsLayout->addWidget(exportButton);
    topControlsLayout->addWidget(reportButton);
    topControlsLayout->addStretch();
    topControlsLayout->addWidget(rowCountLabel);

    filtersLayout->addLayout(topControlsLayout);
    mainLayout->addWidget(filterWidget);

    tableView = new QTableView(this);
    sourceModel = new OlympicTableModel(this);
    proxyModel = new OlympicFilterProxyModel(this);
    proxyModel->setSourceModel(sourceModel);
    tableView->setModel(proxyModel);

    tableView->setSortingEnabled(true);
    tableView->setAlternatingRowColors(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    QHeaderView* header = tableView->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Fixed);

    const int sortIndicatorWidth = 26;

    for(int column = 0; column < sourceModel->columnCount(); ++column) {
        tableView->resizeColumnToContents(column);

        int currentWidth = tableView->columnWidth(column);
        tableView->setColumnWidth(column, currentWidth + sortIndicatorWidth);
    }

    mainLayout->addWidget(tableView);

    connect(addFilterButton, &QPushButton::clicked, this, &OlympicTableView::addFilterRow);
    connect(applyFiltersButton, &QPushButton::clicked, this, &OlympicTableView::applyFilter);
    connect(clearAllButton, &QPushButton::clicked, this, &OlympicTableView::clearFilters);
    connect(proxyModel, &QSortFilterProxyModel::layoutChanged, this, &OlympicTableView::updateRowCountLabel);
    connect(exportButton, &QPushButton::clicked, this, &OlympicTableView::exportData);
    connect(reportButton, &QPushButton::clicked, this, &OlympicTableView::generateReport);

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

void OlympicTableView::exportData() {
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export Data"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("CSV Files (*.csv);;Excel Files (*.xlsx);;JSON Files (*.json);;HTML Files (*.html);;PDF Files (*.pdf)"),
        &selectedFilter
        );

    if (fileName.isEmpty()) {
        return;
    }

    QString format;
    if (selectedFilter.contains("CSV")) {
        format = "csv";
        if (!fileName.endsWith(".csv", Qt::CaseInsensitive)) {
            fileName += ".csv";
        }
    } else if (selectedFilter.contains("Excel")) {
        format = "xlsx";
        if (!fileName.endsWith(".xlsx", Qt::CaseInsensitive)) {
            fileName += ".xlsx";
        }
    } else if (selectedFilter.contains("JSON")) {
        format = "json";
        if (!fileName.endsWith(".json", Qt::CaseInsensitive)) {
            fileName += ".json";
        }
    } else if (selectedFilter.contains("HTML")) {
        format = "html";
        if (!fileName.endsWith(".html", Qt::CaseInsensitive)) {
            fileName += ".html";
        }
    } else if (selectedFilter.contains("PDF")) {
        format = "pdf";
        if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) {
            fileName += ".pdf";
        }
    }

    ExportManager exportManager;
    bool success = exportManager.exportFilteredData(tableView, fileName, format);

    if (success) {
        QMessageBox::information(this, tr("Export Successful"),
                                 tr("Data has been exported to:\n%1").arg(fileName));
    } else {
        QMessageBox::critical(this, tr("Export Failed"),
                              tr("Failed to export data. Please try again."));
    }
}

void OlympicTableView::generateReport() {
    ExportManager exportManager;

    QString title = "Olympic Data Report";
    QString description = QString("This report contains filtered Olympic data showing %1 out of %2 total records.")
                              .arg(proxyModel->rowCount())
                              .arg(sourceModel->rowCount());

    if (!filterRows.isEmpty()) {
        description += "\n\nApplied filters:";
        for (const FilterRow& row : filterRows) {
            QString pattern = row.patternEdit->text();
            if (!pattern.isEmpty()) {
                description += QString("\n- Column '%1': %2")
                                   .arg(row.columnCombo->currentText())
                                   .arg(pattern);
            }
        }
    }

    bool success = exportManager.generateReport(this, title, description, nullptr, tableView);

    if (!success) {
        QMessageBox::critical(this, tr("Report Generation Failed"),
                              tr("Failed to generate report. Please try again."));
    }
}
