#include "olympicgraphview.h"
#include <algorithm>
#include <QDebug>

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
        "Visualização Geográfica"
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
