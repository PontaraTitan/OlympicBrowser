#ifndef OLYMPICGRAPHVIEW_H
#define OLYMPICGRAPHVIEW_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QListWidget>
#include <QSettings>
#include <QRegularExpression>
#include <QToolTip>
#include <QCursor>

#include "database.h"

class OlympicGraphView : public QWidget {
    Q_OBJECT

public:
    enum GraphMode {
        MedalEvolution,
        Demographics,
        CountryComparison,
        GeographicResults
    };

    explicit OlympicGraphView(QWidget *parent = nullptr);
    void setGraphMode(GraphMode mode);

private slots:
    void updateChart();
    void switchChartType();

private:
    enum ChartType {
        LineChart,
        BarChart
    };

private:
    void setupUI();
    void populateCountryList();
    void clearChartData();
    void updateUI();

    QString normalizeCountryName(const QString& rawName);

    void saveSettings();
    void loadSettings();
    void setupMedalEvolutionControls();
    void setupDemographicsControls();
    void setupCountryComparisonControls();
    void setupGeographicControls();

    void createLineChart();
    void createBarChart();
    void createDemographicsChart();
    void createCountryComparisonChart();
    void createGeographicChart();

    QMap<int, int> getMedalCountsByYear(const QString& country, const QString& medalType, const QString& season);
    QStringList getYears(const QString& season);

    void createMedalEvolutionChart() { createLineChart(); }
    void createBarMedalEvolutionChart() { createBarChart(); }

private:
    QChartView* chartView;
    QChart* chart;

    ChartType currentChartType;
    GraphMode currentGraphMode;

    DataBase* db;

    QComboBox* modeCombo;
    QGroupBox* controlsGroup;
    QGridLayout* controlsLayout;
    QComboBox* countryCombo;
    QListWidget* multiCountrySelector;
    QComboBox* attributeCombo;
    QComboBox* yearCombo;
    QComboBox* seasonCombo;
    QComboBox* medalTypeCombo;
    QRadioButton* lineChartRadio;
    QRadioButton* barChartRadio;
    QPushButton* updateButton;

    QMap<QString, QString> countryMapping;
    QMap<QString, QMap<int, int>> medalCache;
};

#endif // OLYMPICGRAPHVIEW_H
