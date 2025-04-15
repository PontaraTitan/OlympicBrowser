#ifndef OLYMPICTABLEVIEW_H
#define OLYMPICTABLEVIEW_H

#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include "olympictablemodel.h"

class OlympicTableView : public QWidget {
    Q_OBJECT

public:
    explicit OlympicTableView(QWidget *parent = nullptr);

private slots:
    void applyFilter();
    void clearFilters();
    void addFilterRow();
    void removeFilterRow();
    void updateRowCountLabel();
    void exportData();
    void generateReport();

private:
    struct FilterRow {
        QComboBox* columnCombo;
        QLineEdit* patternEdit;
        QPushButton* removeButton;
        QWidget* container;
    };

    QTableView* tableView;
    OlympicTableModel* sourceModel;
    OlympicFilterProxyModel* proxyModel;
    QVBoxLayout* mainLayout;
    QVBoxLayout* filtersLayout;
    QVector<FilterRow> filterRows;
    QLabel* rowCountLabel;
    QPushButton* addFilterButton;
    QPushButton* applyFiltersButton;
    QPushButton* clearAllButton;
    QPushButton* exportButton;
    QPushButton* reportButton;

    void setupUI();
    void createFilterRow();
};

#endif // OLYMPICTABLEVIEW_H
