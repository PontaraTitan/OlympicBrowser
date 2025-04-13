#include "olympicgraphview.h"

#include <algorithm>

#include <QDebug>
#include <QHorizontalBarSeries>

OlympicGraphView::OlympicGraphView(QWidget *parent)
    : QWidget(parent)
    , currentChartType(LineChart)
    , currentGraphMode(MedalEvolution)
    , db(DataBase::getInstance())
{
    setupUI();
    populateCountryList();
    loadSettings();
    updateChart();
}

void OlympicGraphView::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QGroupBox* modeGroup = new QGroupBox("Modo de Visualização", this);
    QHBoxLayout* modeLayout = new QHBoxLayout(modeGroup);

    modeCombo = new QComboBox(this);
    modeCombo->addItems({
        "Evolução de Medalhas",
        "Distribuição Demográfica",
        "Comparação entre Países",
        "Visualização Geográfica",
        "Análise Estatística"
    });
    modeLayout->addWidget(modeCombo);

    controlsGroup = new QGroupBox("Controles do Gráfico", this);
    controlsLayout = new QGridLayout(controlsGroup);

    setupMedalEvolutionControls();

    chart = new QChart();
    chartView = new QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);

    mainLayout->addWidget(modeGroup);
    mainLayout->addWidget(controlsGroup);
    mainLayout->addWidget(chartView);

    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
                setGraphMode(static_cast<GraphMode>(index));
            });
}

void OlympicGraphView::populateCountryList()
{
    QMap<QString, bool> normalizedCountries;

    for (const Athlete& athlete : db->getAthletes()) {
        normalizedCountries[normalizeCountryName(athlete.team)] = true;
    }

    QStringList countryList = normalizedCountries.keys();
    std::sort(countryList.begin(), countryList.end());
    countryCombo->addItems(countryList);

    int index = countryList.indexOf("Brazil");
    if (index >= 0) {
        countryCombo->setCurrentIndex(index);
    }
}

QString OlympicGraphView::normalizeCountryName(const QString& rawName)
{
    static QRegularExpression pattern("-(\\d+)$");

    QString normalizedName = rawName;
    QRegularExpressionMatch match = pattern.match(normalizedName);
    if (match.hasMatch()) {
        normalizedName.truncate(match.capturedStart());
    }

    return normalizedName;
}

void OlympicGraphView::updateUI()
{
    QLayoutItem *child;
    while ((child = controlsLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->hide();
            child->widget()->deleteLater();
        }
        delete child;
    }

    countryCombo = nullptr;
    attributeCombo = nullptr;
    seasonCombo = nullptr;
    medalTypeCombo = nullptr;
    seasonCombo = nullptr;
    yearCombo = nullptr;
    lineChartRadio = nullptr;
    barChartRadio = nullptr;
    multiCountrySelector = nullptr;
    updateButton = nullptr;

    switch (currentGraphMode) {
        case MedalEvolution:
            setupMedalEvolutionControls();
            break;
        case Demographics:
            setupDemographicsControls();
            break;
        case CountryComparison:
            setupCountryComparisonControls();
            break;
        case GeographicResults:
            setupGeographicControls();
            break;
        case StatisticalAnalysis:
            setupStatisticalControls();
            break;
    }
}

void OlympicGraphView::setGraphMode(GraphMode mode)
{
    currentGraphMode = mode;
    modeCombo->setCurrentIndex(static_cast<int>(mode));
    updateUI();
    updateChart();
    saveSettings();
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

void OlympicGraphView::updateChart()
{
    clearChartData();
    chart->setAnimationOptions(QChart::SeriesAnimations);

    switch (currentGraphMode) {
        case MedalEvolution:
            if (lineChartRadio->isChecked()) {
                currentChartType = LineChart;
                createLineChart();
            } else {
                currentChartType = BarChart;
                createBarChart();
            }
            break;

        case Demographics:
            createDemographicsChart();
            break;

        case CountryComparison:
            createCountryComparisonChart();
            break;

        case GeographicResults:
            createGeographicChart();
            break;

        case StatisticalAnalysis:
            createStatisticalChart();
            break;
    }

    saveSettings();
}

void OlympicGraphView::switchChartType()
{
    updateChart();
}

QMap<int, int> OlympicGraphView::getMedalCountsByYear(const QString& normalizedCountry, const QString& medalType, const QString& season)
{
    QString cacheKey = normalizedCountry + "|" + medalType + "|" + season;

    if (medalCache.contains(cacheKey)) {
        return medalCache[cacheKey];
    }

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
        QString athleteCountry = normalizeCountryName(athlete.team);
        if (athleteCountry != normalizedCountry) {
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

    medalCache[cacheKey] = medalCounts;

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

void OlympicGraphView::saveSettings()
{
    QSettings settings("OlympicBrowser", "GraphView");
    settings.setValue("GraphMode", currentGraphMode);
    settings.setValue("ChartType", currentChartType);

    switch (currentGraphMode) {
    case MedalEvolution:
    case Demographics:
        if (countryCombo && countryCombo->isVisible() && countryCombo->count() > 0) {
            settings.setValue("SelectedCountry", countryCombo->currentText());
        }
        break;

    case CountryComparison:
        if (multiCountrySelector && multiCountrySelector->isVisible()) {
            QStringList selectedCountries;
            for (QListWidgetItem* item : multiCountrySelector->selectedItems()) {
                selectedCountries.append(item->text());
            }
            settings.setValue("SelectedCountries", selectedCountries);
        }
        break;

    case GeographicResults:
        if (yearCombo && yearCombo->isVisible() && yearCombo->count() > 0) {
            settings.setValue("SelectedYear", yearCombo->currentText());
        }
        break;
    }

    if (seasonCombo && seasonCombo->isVisible() && seasonCombo->count() > 0) {
        settings.setValue("SelectedSeason", seasonCombo->currentIndex());
    }

    if (medalTypeCombo && medalTypeCombo->isVisible() && medalTypeCombo->count() > 0) {
        settings.setValue("SelectedMedalType", medalTypeCombo->currentIndex());
    }
}

void OlympicGraphView::loadSettings()
{
    QSettings settings("OlympicBrowser", "GraphView");

    int mode = settings.value("GraphMode", MedalEvolution).toInt();
    currentGraphMode = static_cast<GraphMode>(mode);

    bool isBarChart = settings.value("ChartType", LineChart).toInt() == BarChart;
    if (lineChartRadio && barChartRadio) {
        lineChartRadio->setChecked(!isBarChart);
        barChartRadio->setChecked(isBarChart);
    }

    modeCombo->setCurrentIndex(mode);
    updateUI();

    switch (currentGraphMode) {
    case MedalEvolution:
    case Demographics:
        if (countryCombo && countryCombo->count() > 0) {
            QString country = settings.value("SelectedCountry").toString();
            if (!country.isEmpty()) {
                int index = countryCombo->findText(country);
                if (index >= 0) {
                    countryCombo->setCurrentIndex(index);
                }
            }
        }
        break;

    case CountryComparison:
        if (multiCountrySelector) {
            QStringList selectedCountries = settings.value("SelectedCountries").toStringList();
            for (int i = 0; i < multiCountrySelector->count(); ++i) {
                QListWidgetItem* item = multiCountrySelector->item(i);
                item->setSelected(selectedCountries.contains(item->text()));
            }
        }
        break;

    case GeographicResults:
        if (yearCombo && yearCombo->count() > 0) {
            QString year = settings.value("SelectedYear").toString();
            if (!year.isEmpty()) {
                int index = yearCombo->findText(year);
                if (index >= 0) {
                    yearCombo->setCurrentIndex(index);
                }
            }
        }
        break;
    }

    if (seasonCombo && seasonCombo->count() > 0) {
        seasonCombo->setCurrentIndex(settings.value("SelectedSeason", 0).toInt());
    }

    if (medalTypeCombo && medalTypeCombo->count() > 0) {
        medalTypeCombo->setCurrentIndex(settings.value("SelectedMedalType", 0).toInt());
    }
}

void OlympicGraphView::setupMedalEvolutionControls()
{
    QLabel* countryLabel = new QLabel("País:", this);
    countryCombo = new QComboBox(this);
    QLabel* medalTypeLabel = new QLabel("Tipo de Medalha:", this);
    medalTypeCombo = new QComboBox(this);
    medalTypeCombo->addItems({"Ouro", "Prata", "Bronze", "Todas"});

    QLabel* seasonLabel = new QLabel("Temporada:", this);
    seasonCombo = new QComboBox(this);
    seasonCombo->addItems({"Verão", "Inverno", "Ambas"});

    QGroupBox* chartTypeGroup = new QGroupBox("Tipo de Gráfico", this);
    QHBoxLayout* chartTypeLayout = new QHBoxLayout(chartTypeGroup);
    lineChartRadio = new QRadioButton("Linha", this);
    barChartRadio = new QRadioButton("Barras", this);
    lineChartRadio->setChecked(true);
    chartTypeLayout->addWidget(lineChartRadio);
    chartTypeLayout->addWidget(barChartRadio);

    updateButton = new QPushButton("Atualizar Gráfico", this);

    controlsLayout->addWidget(countryLabel, 0, 0);
    controlsLayout->addWidget(countryCombo, 0, 1);
    controlsLayout->addWidget(medalTypeLabel, 0, 2);
    controlsLayout->addWidget(medalTypeCombo, 0, 3);
    controlsLayout->addWidget(seasonLabel, 1, 0);
    controlsLayout->addWidget(seasonCombo, 1, 1);
    controlsLayout->addWidget(chartTypeGroup, 1, 2, 1, 2);
    controlsLayout->addWidget(updateButton, 2, 0, 1, 4);

    connect(updateButton, &QPushButton::clicked, this, &OlympicGraphView::updateChart);
    connect(lineChartRadio, &QRadioButton::toggled, this, &OlympicGraphView::switchChartType);

    populateCountryList();
}

void OlympicGraphView::setupDemographicsControls()
{
    QLabel* countryLabel = new QLabel("País:", this);
    countryCombo = new QComboBox(this);

    QLabel* attributeLabel = new QLabel("Atributo:", this);
    attributeCombo = new QComboBox(this);
    attributeCombo->addItems({"Idade", "Altura", "Peso"});

    QLabel* seasonLabel = new QLabel("Temporada:", this);
    seasonCombo = new QComboBox(this);
    seasonCombo->addItems({"Verão", "Inverno", "Ambas"});

    updateButton = new QPushButton("Atualizar Gráfico", this);

    controlsLayout->addWidget(countryLabel, 0, 0);
    controlsLayout->addWidget(countryCombo, 0, 1);
    controlsLayout->addWidget(attributeLabel, 0, 2);
    controlsLayout->addWidget(attributeCombo, 0, 3);
    controlsLayout->addWidget(seasonLabel, 1, 0);
    controlsLayout->addWidget(seasonCombo, 1, 1);
    controlsLayout->addWidget(updateButton, 2, 0, 1, 4);

    connect(updateButton, &QPushButton::clicked, this, &OlympicGraphView::updateChart);

    populateCountryList();
}

void OlympicGraphView::setupCountryComparisonControls()
{
    multiCountrySelector = new QListWidget(this);
    multiCountrySelector->setSelectionMode(QAbstractItemView::MultiSelection);

    QSet<QString> countries;
    for (const Athlete& athlete : db->getAthletes()) {
        countries.insert(normalizeCountryName(athlete.team));
    }

    QStringList countryList = countries.values();
    std::sort(countryList.begin(), countryList.end());

    for (const QString& country : countryList) {
        QListWidgetItem* item = new QListWidgetItem(country);
        multiCountrySelector->addItem(item);
    }

    for (int i = 0; i < multiCountrySelector->count(); ++i) {
        QListWidgetItem* item = multiCountrySelector->item(i);
        if (item->text() == "United States" ||
            item->text() == "China" ||
            item->text() == "Russia" ||
            item->text() == "Brazil") {
            item->setSelected(true);
        }
    }

    QLabel* medalTypeLabel = new QLabel("Tipo de Medalha:", this);
    medalTypeCombo = new QComboBox(this);
    medalTypeCombo->addItems({"Ouro", "Prata", "Bronze", "Todas"});

    QLabel* seasonLabel = new QLabel("Temporada:", this);
    seasonCombo = new QComboBox(this);
    seasonCombo->addItems({"Verão", "Inverno", "Ambas"});

    updateButton = new QPushButton("Atualizar Gráfico", this);

    QLabel* selectCountriesLabel = new QLabel("Selecione os países para comparação:");
    controlsLayout->addWidget(selectCountriesLabel, 0, 0, 1, 2);
    controlsLayout->addWidget(multiCountrySelector, 1, 0, 3, 2);

    controlsLayout->addWidget(medalTypeLabel, 1, 2);
    controlsLayout->addWidget(medalTypeCombo, 1, 3);
    controlsLayout->addWidget(seasonLabel, 2, 2);
    controlsLayout->addWidget(seasonCombo, 2, 3);
    controlsLayout->addWidget(updateButton, 3, 2, 1, 2);

    connect(updateButton, &QPushButton::clicked, this, &OlympicGraphView::updateChart);
}

void OlympicGraphView::setupGeographicControls()
{
    QLabel* gameYearLabel = new QLabel("Ano dos Jogos:", this);
    yearCombo = new QComboBox(this);

    QSet<int> years;
    for (const Athlete& athlete : db->getAthletes()) {
        if (athlete.year > 0) {
            years.insert(athlete.year);
        }
    }

    QList<int> yearsList = years.values();
    std::sort(yearsList.begin(), yearsList.end(), std::greater<int>());

    for (int year : yearsList) {
        yearCombo->addItem(QString::number(year));
    }

    QLabel* seasonLabel = new QLabel("Temporada:", this);
    seasonCombo = new QComboBox(this);
    seasonCombo->addItems({"Verão", "Inverno", "Ambas"});

    QLabel* medalTypeLabel = new QLabel("Tipo de Medalha:", this);
    medalTypeCombo = new QComboBox(this);
    medalTypeCombo->addItems({"Ouro", "Prata", "Bronze", "Todas"});

    updateButton = new QPushButton("Atualizar Gráfico", this);

    controlsLayout->addWidget(gameYearLabel, 0, 0);
    controlsLayout->addWidget(yearCombo, 0, 1);
    controlsLayout->addWidget(seasonLabel, 0, 2);
    controlsLayout->addWidget(seasonCombo, 0, 3);
    controlsLayout->addWidget(medalTypeLabel, 1, 0);
    controlsLayout->addWidget(medalTypeCombo, 1, 1);
    controlsLayout->addWidget(updateButton, 2, 0, 1, 4);

    connect(updateButton, &QPushButton::clicked, this, &OlympicGraphView::updateChart);
}

void OlympicGraphView::setupStatisticalControls()
{
    QLabel* countryLabel = new QLabel("País:", this);
    countryCombo = new QComboBox(this);

    QLabel* analysisTypeLabel = new QLabel("Tipo de Análise:", this);
    QComboBox* analysisTypeCombo = new QComboBox(this);
    analysisTypeCombo->setObjectName("analysisTypeCombo");
    analysisTypeCombo->addItems({
                                 "Tendências de Medalhas ao Longo do Tempo",
                                 "Correlação entre Atributos e Medalhas",
                                 "Distribuição de Medalhas por Esporte"
    });

    QLabel* seasonLabel = new QLabel("Temporada:", this);
    seasonCombo = new QComboBox(this);
    seasonCombo->addItems({"Verão", "Inverno", "Ambas"});

        updateButton = new QPushButton("Gerar Análise", this);

          controlsLayout->addWidget(countryLabel, 0, 0);
    controlsLayout->addWidget(countryCombo, 0, 1);
    controlsLayout->addWidget(analysisTypeLabel, 0, 2);
    controlsLayout->addWidget(analysisTypeCombo, 0, 3);
    controlsLayout->addWidget(seasonLabel, 1, 0);
    controlsLayout->addWidget(seasonCombo, 1, 1);
    controlsLayout->addWidget(updateButton, 2, 0, 1, 4);

    connect(updateButton, &QPushButton::clicked, this, &OlympicGraphView::updateChart);
    connect(analysisTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
                QSettings settings("OlympicBrowser", "GraphView");
                settings.setValue("AnalysisType", index);

                updateChart();
            });

    populateCountryList();

    // Restaurar tipo de análise selecionado anteriormente
    QSettings settings("OlympicBrowser", "GraphView");
    int analysisType = settings.value("AnalysisType", 0).toInt();
    if (analysisType < analysisTypeCombo->count()) {
        analysisTypeCombo->setCurrentIndex(analysisType);
    }
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

    QSet<int> actualYears;
    for (auto it = medalCounts.constBegin(); it != medalCounts.constEnd(); ++it) {
        QPointF point(it.key(), it.value());
        series->append(point);
        actualYears.insert(it.key());
    }

    connect(series, &QLineSeries::hovered, this, [=](const QPointF &point, bool state) {
        if (state && actualYears.contains(static_cast<int>(point.x()))) {
            QToolTip::showText(QCursor::pos(),
                               QString("Ano: %1\nMedalhas: %2").arg(static_cast<int>(point.x())).arg(static_cast<int>(point.y())));
        }
    });

    chart->addSeries(series);

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

    connect(series, &QBarSeries::hovered, this, [=](bool state, int index, QBarSet* barset) {
        if (state && index >= 0 && index < categories.size()) {
            int year = categories.at(index).toInt();
            int medals = barset->at(index);
            QToolTip::showText(QCursor::pos(),
                               QString("Ano: %1\nMedalhas: %2").arg(year).arg(medals));
        }
    });

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
}

void OlympicGraphView::createDemographicsChart()
{
    clearChartData();

    QString country = countryCombo->currentText();
    QString attribute = attributeCombo->currentText();
    QString season;
    switch (seasonCombo->currentIndex()) {
        case 0: season = "Summer"; break;
        case 1: season = "Winter"; break;
        default: season = "All";
    }

    int attributeIndex;
    QString attributeName;
    if (attribute == "Idade") {
        attributeIndex = 0;
        attributeName = "Idade";
    } else if (attribute == "Altura") {
        attributeIndex = 1;
        attributeName = "Altura";
    } else {
        attributeIndex = 2;
        attributeName = "Peso";
    }

    QMap<int, int> valueCounts;
    int total = 0;

    for (const Athlete& athlete : db->getAthletes()) {
        if (normalizeCountryName(athlete.team) != country) {
            continue;
        }

        if (season != "All" && athlete.season != season) {
            continue;
        }

        float value;
        switch (attributeIndex) {
            case 0: value = athlete.age; break;
            case 1: value = athlete.height; break;
            case 2: value = athlete.weight; break;
        }

        if (value <= 0) {
            continue;
        }

        int bucket = qRound(value);
        valueCounts[bucket]++;
        total++;
    }

    QBarSeries *series = new QBarSeries();
    QBarSet *set = new QBarSet(attributeName);

    QStringList categories;
    QVector<QPair<int, int>> sortedValues;

    for (auto it = valueCounts.constBegin(); it != valueCounts.constEnd(); ++it) {
        sortedValues.append(qMakePair(it.key(), it.value()));
    }

    std::sort(sortedValues.begin(), sortedValues.end(),
              [](const QPair<int, int>& a, const QPair<int, int>& b) {
                  return a.first < b.first;
              });

    for (const auto& pair : sortedValues) {
        *set << pair.second;
        categories << QString::number(pair.first);
    }

    connect(series, &QBarSeries::hovered, this, [=](bool state, int index, QBarSet* barset) {
        if (state && index >= 0 && index < categories.size()) {
            int attributeValue = categories.at(index).toInt();
            int count = barset->at(index);
            QToolTip::showText(QCursor::pos(),
                               QString("%1: %2\nAtletas: %3").arg(attributeName).arg(attributeValue).arg(count));
        }
    });

    series->append(set);
    chart->addSeries(series);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Número de Atletas");

    if (!valueCounts.isEmpty()) {
        int maxCount = *std::max_element(valueCounts.begin(), valueCounts.end());
        axisY->setRange(0, maxCount + 1);
    }

    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->setTitle(QString("Distribuição de %1 - %2 (%3 atletas)").arg(attributeName, country).arg(total));
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
}

void OlympicGraphView::createCountryComparisonChart()
{
    clearChartData();

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

    QStringList selectedCountries;
    for (QListWidgetItem* item : multiCountrySelector->selectedItems()) {
        selectedCountries.append(item->text());
    }

    if (selectedCountries.isEmpty()) {
        chart->setTitle("Nenhum país selecionado");
        return;
    }

    QStringList yearLabels;
    QSet<int> allYears;
    QMap<QString, QMap<int, int>> countryData;

    for (const QString& country : selectedCountries) {
        QMap<int, int> medalData = getMedalCountsByYear(country, medalType, season);
        countryData[country] = medalData;

        for (auto it = medalData.constBegin(); it != medalData.constEnd(); ++it) {
            allYears.insert(it.key());
        }
    }

    QList<int> sortedYears = allYears.values();
    std::sort(sortedYears.begin(), sortedYears.end());

    QList<QColor> colors = {
        Qt::red, Qt::blue, Qt::green, Qt::cyan,
        Qt::magenta, Qt::yellow, Qt::black, Qt::darkRed,
        Qt::darkBlue, Qt::darkGreen, Qt::darkCyan
    };

    int colorIndex = 0;

    for (const QString& country : selectedCountries) {
        QLineSeries *series = new QLineSeries();
        series->setName(country);

        QPen pen = series->pen();
        pen.setWidth(2);
        pen.setColor(colors[colorIndex % colors.size()]);
        series->setPen(pen);
        colorIndex++;

        QSet<int> actualYears;
        for (int year : sortedYears) {
            int count = countryData[country].value(year, 0);
            series->append(year, count);
            actualYears.insert(year);
        }

        connect(series, &QLineSeries::hovered, this, [=](const QPointF &point, bool state) {
            if (state && actualYears.contains(static_cast<int>(point.x()))) {
                int year = static_cast<int>(point.x());
                int medals = static_cast<int>(point.y());
                QToolTip::showText(QCursor::pos(),
                                   QString("País: %1\nAno: %2\nMedalhas: %3").arg(country).arg(year).arg(medals));
            }
        });

        chart->addSeries(series);
    }

    QValueAxis *axisX = new QValueAxis();
    QValueAxis *axisY = new QValueAxis();

    axisX->setLabelFormat("%d");
    axisX->setTitleText("Ano");
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Número de Medalhas");

    if (!sortedYears.isEmpty()) {
        axisX->setRange(sortedYears.first(), sortedYears.last());

        int maxMedals = 0;
        for (const auto& countryMedals : countryData) {
            for (int count : countryMedals.values()) {
                maxMedals = qMax(maxMedals, count);
            }
        }

        axisY->setRange(0, maxMedals + 1);
    }

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (QAbstractSeries *series : chart->series()) {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    QString medalName = (medalType == "All") ? "todas medalhas" :
                        (medalType == "Gold") ? "medalhas de ouro" :
                        (medalType == "Silver") ? "medalhas de prata" : "medalhas de bronze";

    chart->setTitle(QString("Comparação de %1 entre países").arg(medalName));
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
}

void OlympicGraphView::createGeographicChart()
{
    clearChartData();

    int selectedYear = yearCombo->currentText().toInt();

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

    QMap<QString, QString> countryContinentMap;

    countryContinentMap["United States"] = "North America";
    countryContinentMap["Canada"] = "North America";
    countryContinentMap["Mexico"] = "North America";

    countryContinentMap["Brazil"] = "South America";
    countryContinentMap["Argentina"] = "South America";
    countryContinentMap["Colombia"] = "South America";

    countryContinentMap["Germany"] = "Europe";
    countryContinentMap["France"] = "Europe";
    countryContinentMap["United Kingdom"] = "Europe";
    countryContinentMap["Italy"] = "Europe";
    countryContinentMap["Spain"] = "Europe";
    countryContinentMap["Russia"] = "Europe";

    countryContinentMap["China"] = "Asia";
    countryContinentMap["Japan"] = "Asia";
    countryContinentMap["South Korea"] = "Asia";
    countryContinentMap["India"] = "Asia";

    countryContinentMap["Australia"] = "Oceania";
    countryContinentMap["New Zealand"] = "Oceania";

    countryContinentMap["South Africa"] = "Africa";
    countryContinentMap["Egypt"] = "Africa";
    countryContinentMap["Kenya"] = "Africa";
    countryContinentMap["Nigeria"] = "Africa";

    QMap<QString, int> continentMedals;
    QMap<QString, QMap<QString, int>> countriesByContinent;

    for (const Athlete& athlete : db->getAthletes()) {
        if (athlete.year != selectedYear) {
            continue;
        }

        if (season != "All" && athlete.season != season) {
            continue;
        }

        bool hasMedal = false;
        if (medalType == "All") {
            hasMedal = !athlete.medal.isEmpty() && athlete.medal != "NA";
        } else {
            hasMedal = (athlete.medal == medalType);
        }

        if (!hasMedal) {
            continue;
        }

        QString normalizedCountry = normalizeCountryName(athlete.team);

        QString continent = countryContinentMap.value(normalizedCountry, "Outros");

        continentMedals[continent]++;
        countriesByContinent[continent][normalizedCountry]++;
    }

    QPieSeries *pieSeries = new QPieSeries();

    for (auto it = continentMedals.constBegin(); it != continentMedals.constEnd(); ++it) {
        QPieSlice *slice = pieSeries->append(it.key(), it.value());
        slice->setLabelVisible(true);

        if (it.key() == "North America") {
            slice->setColor(QColor(255, 0, 0));
        } else if (it.key() == "South America") {
            slice->setColor(QColor(0, 128, 0));
        } else if (it.key() == "Europe") {
            slice->setColor(QColor(0, 0, 255));
        } else if (it.key() == "Asia") {
            slice->setColor(QColor(255, 255, 0));
        } else if (it.key() == "Oceania") {
            slice->setColor(QColor(128, 0, 128));
        } else if (it.key() == "Africa") {
            slice->setColor(QColor(255, 128, 0));
        }

        connect(slice, &QPieSlice::hovered, this, [=](bool state) {
            if (state) {
                QToolTip::showText(QCursor::pos(),
                                   QString("Continente: %1\nMedalhas: %2\nPorcentagem: %3%")
                                       .arg(slice->label())
                                       .arg(static_cast<int>(slice->value()))
                                       .arg(slice->percentage() * 100, 0, 'f', 1));
            }
        });
    }

    chart->addSeries(pieSeries);

    QString medalName = (medalType == "All") ? "todas medalhas" :
                        (medalType == "Gold") ? "medalhas de ouro" :
                        (medalType == "Silver") ? "medalhas de prata" : "medalhas de bronze";

    chart->setTitle(QString("Distribuição de %1 por continente em %2").arg(medalName).arg(selectedYear));
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);
}

void OlympicGraphView::createStatisticalChart()
{
    clearChartData();

    QString country = countryCombo->currentText();
    QString season;
    switch (seasonCombo->currentIndex()) {
    case 0: season = "Summer"; break;
    case 1: season = "Winter"; break;
    default: season = "All";
    }

    QComboBox* analysisTypeCombo = findChild<QComboBox*>("analysisTypeCombo");
    int analysisType = 0;

    if (analysisTypeCombo) {
        analysisType = analysisTypeCombo->currentIndex();
    } else {
        // Fallback para as configurações se o combo não estiver disponível
        QSettings settings("OlympicBrowser", "GraphView");
        analysisType = settings.value("AnalysisType", 0).toInt();
    }

    switch (analysisType) {
    case 0:
        createMedalTrendAnalysis(country, season);
        break;
    case 1:
        createAttributeCorrelationAnalysis(country, season);
        break;
    case 2:
        createSportDistributionAnalysis(country, season);
        break;
    }
}

void OlympicGraphView::createMedalTrendAnalysis(const QString &country, const QString &season)
{
    // Obter dados de medalhas por ano
    QMap<int, int> goldData = getMedalCountsByYear(country, "Gold", season);
    QMap<int, int> silverData = getMedalCountsByYear(country, "Silver", season);
    QMap<int, int> bronzeData = getMedalCountsByYear(country, "Bronze", season);

    // Calcular a linha de tendência linear (regressão simples)
    QList<int> years;
    QList<int> totalMedals;
    QSet<int> allYears;

    // Combinar todos os anos de dados
    for (auto it = goldData.constBegin(); it != goldData.constEnd(); ++it) {
        allYears.insert(it.key());
    }
    for (auto it = silverData.constBegin(); it != silverData.constEnd(); ++it) {
        allYears.insert(it.key());
    }
    for (auto it = bronzeData.constBegin(); it != bronzeData.constEnd(); ++it) {
        allYears.insert(it.key());
    }

    QList<int> sortedYears = allYears.values();
    std::sort(sortedYears.begin(), sortedYears.end());

    // Criar dados para regressão
    for (int year : sortedYears) {
        int total = goldData.value(year, 0) + silverData.value(year, 0) + bronzeData.value(year, 0);
        years.append(year);
        totalMedals.append(total);
    }

    // Calcular médias
    double sumX = 0, sumY = 0;
    for (int i = 0; i < years.size(); i++) {
        sumX += years[i];
        sumY += totalMedals[i];
    }
    double meanX = sumX / years.size();
    double meanY = sumY / years.size();

    // Calcular coeficientes de regressão
    double numerator = 0, denominator = 0;
    for (int i = 0; i < years.size(); i++) {
        numerator += (years[i] - meanX) * (totalMedals[i] - meanY);
        denominator += (years[i] - meanX) * (years[i] - meanX);
    }

    double slope = 0;
    if (denominator != 0) {
        slope = numerator / denominator;
    }
    double intercept = meanY - slope * meanX;

    // Criar séries para o gráfico
    QLineSeries *goldSeries = new QLineSeries();
    goldSeries->setName("Ouro");

    QLineSeries *silverSeries = new QLineSeries();
    silverSeries->setName("Prata");

    QLineSeries *bronzeSeries = new QLineSeries();
    bronzeSeries->setName("Bronze");

    QLineSeries *totalSeries = new QLineSeries();
    totalSeries->setName("Total");

    QLineSeries *trendSeries = new QLineSeries();
    trendSeries->setName("Tendência");
        QPen trendPen = trendSeries->pen();
    trendPen.setStyle(Qt::DashLine);
    trendPen.setWidth(2);
    trendPen.setColor(Qt::red);
    trendSeries->setPen(trendPen);

    // Preencher séries com dados
    for (int year : sortedYears) {
        goldSeries->append(year, goldData.value(year, 0));
        silverSeries->append(year, silverData.value(year, 0));
        bronzeSeries->append(year, bronzeData.value(year, 0));
        int total = goldData.value(year, 0) + silverData.value(year, 0) + bronzeData.value(year, 0);
        totalSeries->append(year, total);

        // Adicionar valor da linha de tendência
        double trendValue = intercept + slope * year;
        trendSeries->append(year, trendValue);
    }

    // Adicionar séries ao gráfico
    chart->addSeries(goldSeries);
    chart->addSeries(silverSeries);
    chart->addSeries(bronzeSeries);
    chart->addSeries(totalSeries);
    chart->addSeries(trendSeries);

    // Configurar eixos
    QValueAxis *axisX = new QValueAxis();
    QValueAxis *axisY = new QValueAxis();

    axisX->setLabelFormat("%d");
    axisX->setTitleText("Ano");
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Número de Medalhas");

        if (!sortedYears.isEmpty()) {
        axisX->setRange(sortedYears.first(), sortedYears.last());

        // Encontrar valor máximo em todas as séries
        int maxY = 0;
        for (int year : sortedYears) {
            int total = goldData.value(year, 0) + silverData.value(year, 0) + bronzeData.value(year, 0);
            maxY = qMax(maxY, total);
        }

        // Considerar também o valor máximo na linha de tendência
        double maxTrend = intercept + slope * sortedYears.last();
        maxY = qMax(maxY, static_cast<int>(maxTrend) + 1);

        axisY->setRange(0, maxY);
    }

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    goldSeries->attachAxis(axisX);
    goldSeries->attachAxis(axisY);
    silverSeries->attachAxis(axisX);
    silverSeries->attachAxis(axisY);
    bronzeSeries->attachAxis(axisX);
    bronzeSeries->attachAxis(axisY);
    totalSeries->attachAxis(axisX);
    totalSeries->attachAxis(axisY);
    trendSeries->attachAxis(axisX);
    trendSeries->attachAxis(axisY);

    // Adicionar tooltips
    for (QLineSeries* series : {goldSeries, silverSeries, bronzeSeries, totalSeries}) {
        connect(series, &QLineSeries::hovered, this, [=](const QPointF &point, bool state) {
            if (state) {
                QToolTip::showText(QCursor::pos(),
                                   QString("Ano: %1\n%2: %3")
                                       .arg(static_cast<int>(point.x()))
                                       .arg(series->name())
                                       .arg(static_cast<int>(point.y())));
            }
        });
    }

    // Adicionar estatísticas ao título
    double averageMedals = meanY;
    double growthRate = slope;
    QString trendDirection = (slope > 0) ? "crescente" : (slope < 0) ? "decrescente" : "estável";

    chart->setTitle(QString("Análise de Tendência - %1\nMédia: %2 medalhas/ano | Tendência: %3 (%4 medalhas/ano)")
                        .arg(country, QString::number(averageMedals, 'f', 1), trendDirection, QString::number(qAbs(growthRate), 'f', 2)));

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
}

void OlympicGraphView::createAttributeCorrelationAnalysis(const QString &country, const QString &season)
{
    // Vamos analisar a correlação entre idade, altura, peso e medalhas
    QMap<QString, QVector<float>> attributes;
    QVector<float> ages;
    QVector<float> heights;
    QVector<float> weights;
    QVector<float> hasMedal;

    for (const Athlete& athlete : db->getAthletes()) {
        if (normalizeCountryName(athlete.team) != country) {
            continue;
        }

        if (season != "All" && athlete.season != season) {
            continue;
        }

        // Ignorar atletas com dados incompletos
        if (athlete.age <= 0 || athlete.height <= 0 || athlete.weight <= 0) {
            continue;
        }

        ages.append(athlete.age);
        heights.append(athlete.height);
        weights.append(athlete.weight);

        // 1 se tem medalha, 0 se não
        float medalValue = (!athlete.medal.isEmpty() && athlete.medal != "NA") ? 1.0f : 0.0f;
        hasMedal.append(medalValue);
    }

    attributes["Idade"] = ages;
    attributes["Altura"] = heights;
    attributes["Peso"] = weights;

    // Calcular correlações
    QMap<QString, float> correlations;
    for (auto it = attributes.constBegin(); it != attributes.constEnd(); ++it) {
        const QString& attributeName = it.key();
        const QVector<float>& attributeValues = it.value();

        // Verificar se temos dados suficientes
        if (attributeValues.size() < 2) {
            correlations[attributeName] = 0.0f;
            continue;
        }

        // Calcular correlação
        float sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0, sumY2 = 0;
        int n = attributeValues.size();

        for (int i = 0; i < n; i++) {
            float x = attributeValues[i];
            float y = hasMedal[i];

            sumX += x;
            sumY += y;
            sumXY += x * y;
            sumX2 += x * x;
            sumY2 += y * y;
        }

        float correlation = 0.0f;
        float numerator = n * sumXY - sumX * sumY;
        float denominator = sqrt((n * sumX2 - sumX * sumX) * (n * sumY2 - sumY * sumY));

        if (denominator != 0) {
            correlation = numerator / denominator;
        }

        correlations[attributeName] = correlation;
    }

    // Criar gráfico de barras para correlações
    QBarSeries *series = new QBarSeries();
    QBarSet *correlationSet = new QBarSet("Correlação com Medalhas");

    // Adicionar valores absolutos para barras
    QStringList categories;
    for (auto it = correlations.constBegin(); it != correlations.constEnd(); ++it) {
        *correlationSet << qAbs(it.value());
        categories << it.key();
    }

    series->append(correlationSet);
    chart->addSeries(series);

    // Configurar eixos
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%.2f");
    axisY->setTitleText("Coeficiente de Correlação (valor absoluto)");
    axisY->setRange(0, 1.0);

    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Adicionar tooltip
    connect(series, &QBarSeries::hovered, this, [=](bool state, int index, QBarSet* barset) {
        if (state && index >= 0 && index < categories.size()) {
            QString attribute = categories.at(index);
            float correlation = correlations.value(attribute);
            float absCorrelation = qAbs(correlation);
            QString direction = correlation > 0 ? "positiva" : correlation < 0 ? "negativa" : "neutra";
            QString strength;

            if (absCorrelation < 0.3) strength = "fraca";
            else if (absCorrelation < 0.7) strength = "moderada";
            else strength = "forte";

            QString interpretation;
            if (attribute == "Idade") {
                if (correlation > 0) interpretation = "Atletas mais velhos tendem a ganhar mais medalhas";
                else if (correlation < 0) interpretation = "Atletas mais jovens tendem a ganhar mais medalhas";
            } else if (attribute == "Altura") {
                if (correlation > 0) interpretation = "Atletas mais altos tendem a ganhar mais medalhas";
                else if (correlation < 0) interpretation = "Atletas mais baixos tendem a ganhar mais medalhas";
            } else if (attribute == "Peso") {
                if (correlation > 0) interpretation = "Atletas mais pesados tendem a ganhar mais medalhas";
                else if (correlation < 0) interpretation = "Atletas mais leves tendem a ganhar mais medalhas";
            }

            QToolTip::showText(QCursor::pos(),
                              QString("%1: %2\nCorrelação %3 %4\n%5")
                              .arg(attribute)
                              .arg(QString::number(correlation, 'f', 3))
                              .arg(direction)
                              .arg(strength)
                              .arg(interpretation));
        }
    });

    // Calcular estatísticas adicionais
    int totalAthletes = ages.size();
    int medalCount = 0;
    float avgAge = 0, avgHeight = 0, avgWeight = 0;
    float medalistAvgAge = 0, medalistAvgHeight = 0, medalistAvgWeight = 0;

    for (int i = 0; i < totalAthletes; i++) {
        avgAge += ages[i];
        avgHeight += heights[i];
        avgWeight += weights[i];

        if (hasMedal[i] > 0) {
            medalCount++;
            medalistAvgAge += ages[i];
            medalistAvgHeight += heights[i];
            medalistAvgWeight += weights[i];
        }
    }

    if (totalAthletes > 0) {
        avgAge /= totalAthletes;
        avgHeight /= totalAthletes;
        avgWeight /= totalAthletes;
    }

    if (medalCount > 0) {
        medalistAvgAge /= medalCount;
        medalistAvgHeight /= medalCount;
        medalistAvgWeight /= medalCount;
    }

    chart->setTitle(QString("Correlação entre Atributos e Medalhas - %1\n"
                            "Total de atletas: %2 | Com medalhas: %3 (%4%)")
                        .arg(country)
                        .arg(QString::number(totalAthletes))
                        .arg(QString::number(medalCount))
                        .arg(QString::number(totalAthletes > 0 ? (medalCount * 100.0 / totalAthletes) : 0.0, 'f', 1)));
    chart->legend()->setAlignment(Qt::AlignBottom);
}

void OlympicGraphView::createSportDistributionAnalysis(const QString &country, const QString &season)
{
    // Analisar distribuição de medalhas por esporte
    QMap<QString, int> sportMedals;
    QMap<QString, int> sportAthletes;

    for (const Athlete& athlete : db->getAthletes()) {
        if (normalizeCountryName(athlete.team) != country) {
            continue;
        }

        if (season != "All" && athlete.season != season) {
            continue;
        }

        // Contar atletas por esporte
        sportAthletes[athlete.sport]++;

        // Contar medalhas por esporte
        if (!athlete.medal.isEmpty() && athlete.medal != "NA") {
            sportMedals[athlete.sport]++;
        }
    }

    // Calcular eficiência por esporte (medalhas/atletas)
    QMap<QString, float> sportEfficiency;
    QVector<QPair<QString, float>> sortedEfficiency;

    for (auto it = sportAthletes.constBegin(); it != sportAthletes.constEnd(); ++it) {
        const QString& sport = it.key();
        int athletes = it.value();
        int medals = sportMedals.value(sport, 0);

        if (athletes > 0) {
            float efficiency = static_cast<float>(medals) / athletes;
            sportEfficiency[sport] = efficiency;

            // Adicionar apenas esportes com pelo menos 5 atletas para evitar estatísticas enviesadas
            if (athletes >= 5) {
                sortedEfficiency.append(qMakePair(sport, efficiency));
            }
        }
    }

    // Ordenar por eficiência
    std::sort(sortedEfficiency.begin(), sortedEfficiency.end(),
              [](const QPair<QString, float>& a, const QPair<QString, float>& b) {
                  return a.second > b.second;
              });

    // Limitar para os 10 melhores esportes
    int maxSports = qMin(10, sortedEfficiency.size());

    // Criar gráfico de barras horizontais para eficiência
    QHorizontalBarSeries *series = new QHorizontalBarSeries();
    QBarSet *efficiencySet = new QBarSet("Eficiência (Medalhas/Atleta)");

    QStringList categories;
    for (int i = 0; i < maxSports; i++) {
        const QPair<QString, float>& pair = sortedEfficiency[i];
        *efficiencySet << pair.second;

        // Adicionar número de atletas e medalhas ao nome do esporte
        int athletes = sportAthletes.value(pair.first, 0);
        int medals = sportMedals.value(pair.first, 0);
        categories << QString("%1 (%2 atletas, %3 medalhas)")
                      .arg(pair.first)
                      .arg(athletes)
                      .arg(medals);
    }

    series->append(efficiencySet);
    chart->addSeries(series);

    // Configurar eixos horizontais
    QBarCategoryAxis *axisY = new QBarCategoryAxis();
    axisY->append(categories);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QValueAxis *axisX = new QValueAxis();
    axisX->setLabelFormat("%.2f");
    axisX->setTitleText("Eficiência (Medalhas por Atleta)");

    // Determinar o valor máximo para o eixo X
    float maxEfficiency = 0.0f;
    if (!sortedEfficiency.isEmpty()) {
        maxEfficiency = sortedEfficiency.first().second;
    }
    axisX->setRange(0, maxEfficiency * 1.1); // Adicionar 10% para espaço

    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Adicionar tooltip
    connect(series, &QBarSeries::hovered, this, [=](bool state, int index, QBarSet* barset) {
        if (state && index >= 0 && index < maxSports) {
            const QPair<QString, float>& pair = sortedEfficiency[index];
            QString sport = pair.first;
            float efficiency = pair.second;
            int athletes = sportAthletes.value(sport, 0);
            int medals = sportMedals.value(sport, 0);

            QToolTip::showText(QCursor::pos(),
                              QString("Esporte: %1\nAtletas: %2\nMedalhas: %3\nEficiência: %4\n"
                                     "Posição no ranking: %5 de %6")
                                   .arg(sport)
                                   .arg(QString::number(athletes))
                                   .arg(QString::number(medals))
                                   .arg(QString::number(efficiency, 'f', 3))
                                   .arg(QString::number(index + 1))
                                   .arg(QString::number(maxSports)));
        }
    });

    // Calcular estatísticas gerais
    int totalAthletes = 0;
    int totalMedals = 0;
    int totalSports = sportAthletes.size();

    for (int count : sportAthletes.values()) {
        totalAthletes += count;
    }

    for (int count : sportMedals.values()) {
        totalMedals += count;
    }

    float overallEfficiency = totalAthletes > 0 ?
                             static_cast<float>(totalMedals) / totalAthletes : 0.0f;

    chart->setTitle(QString("Eficiência por Esporte - %1\n"
                            "Total: %2 esportes, %3 atletas, %4 medalhas, Eficiência geral: %5")
                                .arg(country)
                                .arg(totalSports)
                                .arg(totalAthletes)
                                .arg(totalMedals)
                                .arg(QString::number(overallEfficiency, 'f', 3)));

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
}
