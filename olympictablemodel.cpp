#include "olympictablemodel.h"

OlympicTableModel::OlympicTableModel(QObject *parent)
    : QAbstractTableModel(parent)
    , db(DataBase::getInstance())
{
    headers << "ID" << "Name" << "Sex" << "Age" << "Height" << "Weight"
            << "Team" << "NOC" << "Games" << "Year" << "Season" << "City"
            << "Sport" << "Event" << "Medal";

    connect(db, &DataBase::dataChanged, this, &OlympicTableModel::refreshData);
}

int OlympicTableModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return db->getAthletes().size();
}

int OlympicTableModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return headers.size();
}

QVariant OlympicTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const Athlete& athlete = db->getAthletes().at(index.row());

    switch (index.column()) {
    case 0: return athlete.id;
    case 1: return athlete.name;
    case 2: return athlete.sex;
    case 3: return athlete.age;
    case 4: return athlete.height;
    case 5: return athlete.weight;
    case 6: return athlete.team;
    case 7: return athlete.noc;
    case 8: return athlete.games;
    case 9: return athlete.year;
    case 10: return athlete.season;
    case 11: return athlete.city;
    case 12: return athlete.sport;
    case 13: return athlete.event;
    case 14: return athlete.medal;
    default: return QVariant();
    }
}

QVariant OlympicTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal && section < headers.size())
        return headers.at(section);

    return QVariant();
}

void OlympicTableModel::refreshData() {
    beginResetModel();
    endResetModel();
}

OlympicFilterProxyModel::OlympicFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortCaseSensitivity(Qt::CaseInsensitive);
}

void OlympicFilterProxyModel::setColumnFilter(int column, const QString& pattern) {
    if (pattern.isEmpty())
        columnFilters.remove(column);
    else
        columnFilters[column] = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
    invalidateFilter();
}

void OlympicFilterProxyModel::clearFilters() {
    columnFilters.clear();
    invalidateFilter();
}

bool OlympicFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    for (auto it = columnFilters.constBegin(); it != columnFilters.constEnd(); ++it) {
        QModelIndex index = sourceModel()->index(sourceRow, it.key(), sourceParent);
        QString text = sourceModel()->data(index, Qt::DisplayRole).toString();
        if (!text.contains(it.value()))
            return false;
    }
    return true;
}
