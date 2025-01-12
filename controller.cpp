#include "controller.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QStringList>

Controller::Controller(QObject *parent)
    : QObject(parent), db(DataBase::getInstance())
{
}

bool Controller::loadCSV(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << filePath;
        emit dataLoaded(false);
        return false;
    }

    db->clear();
    QTextStream in(&file);

    // Skip header line
    if (!in.atEnd()) {
        in.readLine();
    }

    int lineCount = 0;
    int totalLines = 271116;

    while (!in.atEnd()) {
        QString line = in.readLine();
        QVector<QString> fields;
        QString currentField;
        bool inQuotes = false;

        // Parse CSV line with proper quote handling
        for (int i = 0; i < line.length(); ++i) {
            QChar currentChar = line[i];

            if (currentChar == '\"') {
                inQuotes = !inQuotes;
            }
            else if (currentChar == ',' && !inQuotes) {
                // Remove surrounding quotes if they exist
                if (currentField.startsWith('\"') && currentField.endsWith('\"')) {
                    currentField = currentField.mid(1, currentField.length() - 2);
                }
                fields.append(currentField.trimmed());
                currentField.clear();
            }
            else {
                currentField += currentChar;
            }
        }

        // Add the last field
        if (currentField.startsWith('\"') && currentField.endsWith('\"')) {
            currentField = currentField.mid(1, currentField.length() - 2);
        }
        fields.append(currentField.trimmed());

        if (fields.size() >= 15) {
            Athlete athlete;
            athlete.id = fields[0].toInt();
            athlete.name = fields[1];
            athlete.sex = fields[2];
            athlete.age = fields[3] == "NA" ? 0.0f : fields[3].toFloat();
            athlete.height = fields[4] == "NA" ? 0.0f : fields[4].toFloat();
            athlete.weight = fields[5] == "NA" ? 0.0f : fields[5].toFloat();
            athlete.team = fields[6];
            athlete.noc = fields[7];
            athlete.games = fields[8];
            athlete.year = fields[9].toInt();
            athlete.season = fields[10];
            athlete.city = fields[11];
            athlete.sport = fields[12];
            athlete.event = fields[13];
            athlete.medal = fields[14] == "NA" ? "" : fields[14];

            db->addAthlete(athlete);
        }

        lineCount++;
        if (lineCount % 1000 == 0) {  // Update progress every 1000 lines for better performance
            emit progressUpdated((lineCount * 100) / totalLines);
            QApplication::processEvents();  // Keep UI responsive
        }
    }

    file.close();
    emit dataLoaded(true);
    return true;
}
