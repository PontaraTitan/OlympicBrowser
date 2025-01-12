#include "database.h"

DataBase* DataBase::instance = nullptr;

DataBase::DataBase(QObject *parent) : QObject(parent) {
}

DataBase* DataBase::getInstance() {
    if (instance == nullptr) {
        instance = new DataBase();
    }
    return instance;
}

void DataBase::addAthlete(const Athlete& athlete) {
    athletes.append(athlete);
    emit dataChanged();
}

void DataBase::clear() {
    athletes.clear();
    emit dataChanged();
}
