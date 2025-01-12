#ifndef OLYMPICTABLEMODEL_H
#define OLYMPICTABLEMODEL_H

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QVector>
#include <QRegularExpression>
#include "database.h"

class OlympicTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit OlympicTableModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void refreshData();

private:
    DataBase* db;
    QStringList headers;
};

// Custom proxy model for multi-column filtering
class OlympicFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit OlympicFilterProxyModel(QObject *parent = nullptr);
    void setColumnFilter(int column, const QString& pattern);
    void clearFilters();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QMap<int, QRegularExpression> columnFilters;
};

#endif // OLYMPICTABLEMODEL_H
