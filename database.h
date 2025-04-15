#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QDir>

struct Athlete {
    int id;
    QString name;
    QString sex;
    float age;
    float height;
    float weight;
    QString team;
    QString noc;
    QString games;
    int year;
    QString season;
    QString city;
    QString sport;
    QString event;
    QString medal;
};

class DataBase : public QObject {
    Q_OBJECT
private:
    QVector<Athlete> athletes;
    static DataBase* instance;
    DataBase(QObject *parent = nullptr);

public:
    static DataBase* getInstance();
    void addAthlete(const Athlete& athlete);
    void clear();
    const QVector<Athlete>& getAthletes() const { return athletes; }

signals:
    void dataChanged();
};

#endif // DATABASE_H
