// olympicgraphview.h
#ifndef OLYMPICGRAPHVIEW_H
#define OLYMPICGRAPHVIEW_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarCategoryAxis>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>

class DataBase;

class OlympicGraphView : public QWidget {
    Q_OBJECT

public:
    explicit OlympicGraphView(QWidget *parent = nullptr);

private slots:
    void updateChart();
    void switchChartType();

private:
    void setupUI();
    void populateCountryList();
    void createLineChart();
    void createBarChart();
    void clearChartData();

    QString normalizeCountryName(const QString& rawName);

    // Helper functions to get data
    QMap<int, int> getMedalCountsByYear(const QString& normalizedCountry, const QString& medalType, const QString& season);
    QStringList getYears(const QString& season);


private:
    enum ChartType {
        LineChart,
        BarChart
    };

    QChartView* chartView;
    QChart* chart;

    QComboBox* countryCombo;
    QComboBox* medalTypeCombo;
    QRadioButton* lineChartRadio;
    QRadioButton* barChartRadio;
    QComboBox* seasonCombo;
    QPushButton* updateButton;

    ChartType currentChartType;
    DataBase* db;

    QMap<QString, QString> countryMapping;
};

#endif // OLYMPICGRAPHVIEW_H
