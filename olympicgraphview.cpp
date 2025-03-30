#include <QtCharts/QBarSet>
#include <QtCharts/QAbstractAxis>
#include <QDebug>

#include "olympicgraphview.h"
#include "database.h"

OlympicGraphView::OlympicGraphView(QWidget *parent)
    : QWidget(parent)
    , currentChartType(LineChart)
    , db(DataBase::getInstance())
{
    setupUI();
    populateCountryList();
    updateChart();
}

void OlympicGraphView::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QGroupBox* controlsGroup = new QGroupBox("Controles do Gráfico", this);
    QGridLayout* controlsLayout = new QGridLayout(controlsGroup);

    QGroupBox* chartTypeGroup = new QGroupBox("Tipo de Gráfico", this);
    QHBoxLayout* chartTypeLayout = new QHBoxLayout(chartTypeGroup);
    lineChartRadio = new QRadioButton("Linha", this);
    barChartRadio = new QRadioButton("Barras", this);
    lineChartRadio->setChecked(true);
    chartTypeLayout->addWidget(lineChartRadio);
    chartTypeLayout->addWidget(barChartRadio);

    QLabel* countryLabel = new QLabel("País:", this);
    countryCombo = new QComboBox(this);
    QLabel* medalTypeLabel = new QLabel("Tipo de Medalha:", this);
    medalTypeCombo = new QComboBox(this);
    medalTypeCombo->addItems({"Ouro", "Prata", "Bronze", "Todas"});

    QLabel* seasonLabel = new QLabel("Temporada:", this);
    seasonCombo = new QComboBox(this);
    seasonCombo->addItems({"Verão", "Inverno", "Ambas"});

    updateButton = new QPushButton("Atualizar Gráfico", this);

    controlsLayout->addWidget(countryLabel, 0, 0);
    controlsLayout->addWidget(countryCombo, 0, 1);
    controlsLayout->addWidget(medalTypeLabel, 0, 2);
    controlsLayout->addWidget(medalTypeCombo, 0, 3);
    controlsLayout->addWidget(seasonLabel, 1, 0);
    controlsLayout->addWidget(seasonCombo, 1, 1);
    controlsLayout->addWidget(chartTypeGroup, 1, 2, 1, 2);
    controlsLayout->addWidget(updateButton, 2, 0, 1, 4);

    chart = new QChart();
    chartView = new QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);

    mainLayout->addWidget(controlsGroup);
    mainLayout->addWidget(chartView);

    connect(updateButton, &QPushButton::clicked, this, &OlympicGraphView::updateChart);
    connect(lineChartRadio, &QRadioButton::toggled, this, &OlympicGraphView::switchChartType);
}

void OlympicGraphView::populateCountryList()
{
    QSet<QString> countries;
    for (const Athlete& athlete : db->getAthletes()) {
        countries.insert(athlete.team);
    }

    QStringList countryList = countries.values();
    std::sort(countryList.begin(), countryList.end());
    countryCombo->addItems(countryList);

    int index = countryList.indexOf("Brazil");
    if (index >= 0) {
        countryCombo->setCurrentIndex(index);
    }
}

void OlympicGraphView::updateChart()
{
    if (lineChartRadio->isChecked()) {
        currentChartType = LineChart;
        createLineChart();
    } else {
        currentChartType = BarChart;
        createBarChart();
    }
}

void OlympicGraphView::switchChartType()
{
    updateChart();
}

void OlympicGraphView::createLineChart()
{
    clearChartData();

    QString country = countryCombo->currentText();
    QString medalType;
    switch (medalTypeCombo->currentIndex()) {
        case 0: medalType = "Gold"; break;
        case 1: medalType = "Silver"; break;
        case 2: medalType = "Bronze"; break;
        default: medalType = "All";
    }

    QString season;
    switch (seasonCombo->currentIndex()) {
        case 0: season = "Summer"; break;
        case 1: season = "Winter"; break;
        default: season = "All";
    }

    QMap<int, int> medalCounts = getMedalCountsByYear(country, medalType, season);

    QLineSeries *series = new QLineSeries();
    series->setName(country + " - " + medalTypeCombo->currentText());

    for (auto it = medalCounts.constBegin(); it != medalCounts.constEnd(); ++it) {
        series->append(it.key(), it.value());
    }

    chart->addSeries(series);

    QStringList years;
    for (auto year : medalCounts.keys()) {
        years.append(QString::number(year));
    }

    QValueAxis *axisX = new QValueAxis();
    QValueAxis *axisY = new QValueAxis();

    axisX->setLabelFormat("%d");
    axisX->setTitleText("Ano");
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Número de Medalhas");

    if (!medalCounts.isEmpty()) {
        axisX->setRange(medalCounts.firstKey(), medalCounts.lastKey());
        int maxMedals = *std::max_element(medalCounts.begin(), medalCounts.end());
        axisY->setRange(0, maxMedals + 1);
    }

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chart->setTitle("Evolução de Medalhas - " + country);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    chartView->setChart(chart);
}

void OlympicGraphView::createBarChart()
{
    clearChartData();

    QString country = countryCombo->currentText();
    QString medalType;
    switch (medalTypeCombo->currentIndex()) {
        case 0: medalType = "Gold"; break;
        case 1: medalType = "Silver"; break;
        case 2: medalType = "Bronze"; break;
        default: medalType = "All";
    }

    QString season;
    switch (seasonCombo->currentIndex()) {
        case 0: season = "Summer"; break;
        case 1: season = "Winter"; break;
        default: season = "All";
    }

    QMap<int, int> medalCounts = getMedalCountsByYear(country, medalType, season);

    QBarSeries *series = new QBarSeries();

    QBarSet *set = new QBarSet(medalTypeCombo->currentText());

    QStringList categories;
    for (auto it = medalCounts.constBegin(); it != medalCounts.constEnd(); ++it) {
        *set << it.value();
        categories << QString::number(it.key());
    }

    series->append(set);
    chart->addSeries(series);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Número de Medalhas");

    if (!medalCounts.isEmpty()) {
        int maxMedals = *std::max_element(medalCounts.begin(), medalCounts.end());
        axisY->setRange(0, maxMedals + 1);
    }

    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->setTitle("Medalhas por Ano - " + country);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    chartView->setChart(chart);
}

void OlympicGraphView::clearChartData()
{
    chart->removeAllSeries();

    QList<QAbstractAxis*> axes = chart->axes();
    for (auto axis : axes) {
        chart->removeAxis(axis);
        delete axis;
    }

    chart->setTitle("");
}

QMap<int, int> OlympicGraphView::getMedalCountsByYear(const QString& country, const QString& medalType, const QString& season)
{
    QMap<int, int> medalCounts;

    QSet<int> yearsSet;
    for (const Athlete& athlete : db->getAthletes()) {
        if ((season == "All" || athlete.season == season) &&
            athlete.year > 0) {
            yearsSet.insert(athlete.year);
        }
    }

    QList<int> years = yearsSet.values();
    std::sort(years.begin(), years.end());

    for (int year : years) {
        medalCounts[year] = 0;
    }

    for (const Athlete& athlete : db->getAthletes()) {
        if (athlete.team != country) {
            continue;
        }

        if (season != "All" && athlete.season != season) {
            continue;
        }

        if (medalType == "All") {
            if (!athlete.medal.isEmpty() && athlete.medal != "NA") {
                medalCounts[athlete.year]++;
            }
        }
        else if (athlete.medal == medalType) {
            medalCounts[athlete.year]++;
        }
    }

    return medalCounts;
}

QStringList OlympicGraphView::getYears(const QString& season)
{
    QSet<int> uniqueYears;

    for (const Athlete& athlete : db->getAthletes()) {
        if (season == "All" || athlete.season == season) {
            uniqueYears.insert(athlete.year);
        }
    }

    QList<int> yearsList = uniqueYears.values();
    std::sort(yearsList.begin(), yearsList.end());

    QStringList result;
    for (int year : yearsList) {
        result << QString::number(year);
    }

    return result;
}
