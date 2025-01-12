#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "database.h"
#include <QObject>
#include <QString>
#include <QDir>
#include <QDebug>

class Controller : public QObject {
    Q_OBJECT
private:
    DataBase* db;

public:
    explicit Controller(QObject *parent = nullptr);
    bool loadCSV(const QString& filePath);

signals:
    void dataLoaded(bool success);
    void progressUpdated(int progress);
};

#endif // CONTROLLER_H
