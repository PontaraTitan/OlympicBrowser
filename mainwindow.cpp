#include <QMessageBox>

#include "mainwindow.h"
#include "reportdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , controller(new Controller(this))
    , currentViewMode(TableMode)
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);

    statusLabel = new QLabel("Loading data...", this);
    progressBar = new QProgressBar(this);

    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(progressBar);

    progressBar->setVisible(true);
    setupMenu();

    connect(controller, &Controller::dataLoaded, this, &MainWindow::handleDataLoaded);
    connect(controller, &Controller::progressUpdated, this, &MainWindow::updateProgress);

    resize(2400, 1200);

    // Load data automatically
    QString filePath = QDir::currentPath() + "/datasets/athlete_events.csv";
    controller->loadCSV(filePath);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupMenu()
{
    menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    viewMenu = menuBar->addMenu("Modo de Visualização");

    tableViewAction = viewMenu->addAction("Tabela");
    graphViewAction = viewMenu->addAction("Visualização Gráfica");

    tableViewAction->setCheckable(true);
    graphViewAction->setCheckable(true);
    tableViewAction->setChecked(true);

    connect(tableViewAction, &QAction::triggered, this, [this]() { switchView(TableMode); });
    connect(graphViewAction, &QAction::triggered, this, [this]() { switchView(GraphMode); });

    graphModeMenu = new QMenu("Modo de Gráfico", this);

    medalEvolutionAction = graphModeMenu->addAction("Evolução de Medalhas");
    demographicsAction = graphModeMenu->addAction("Distribuição Demográfica");
    countryComparisonAction = graphModeMenu->addAction("Comparação entre Países");
    geographicAction = graphModeMenu->addAction("Visualização Geográfica");
    statisticalAction = graphModeMenu->addAction("Análise Estatística");

    QList<QAction*> graphModeActions = {
        medalEvolutionAction, demographicsAction,
        countryComparisonAction, geographicAction,
        statisticalAction
    };

    QActionGroup* graphModeGroup = new QActionGroup(this);
    for (QAction* action : graphModeActions) {
        action->setCheckable(true);
        graphModeGroup->addAction(action);
    }
    medalEvolutionAction->setChecked(true);

    connect(medalEvolutionAction, &QAction::triggered, this, [this]() { onGraphModeChanged(0); });
    connect(demographicsAction, &QAction::triggered, this, [this]() { onGraphModeChanged(1); });
    connect(countryComparisonAction, &QAction::triggered, this, [this]() { onGraphModeChanged(2); });
    connect(geographicAction, &QAction::triggered, this, [this]() { onGraphModeChanged(3); });
    connect(statisticalAction, &QAction::triggered, this, [this]() { onGraphModeChanged(4); });

    menuBar->addMenu(graphModeMenu);

    QMenu* exportMenu = menuBar->addMenu("Exportar");

    combinedReportAction = exportMenu->addAction("Gerar Relatório Combinado");
    connect(combinedReportAction, &QAction::triggered, this, &MainWindow::generateCombinedReport);
}

void MainWindow::handleDataLoaded(bool success) {
    progressBar->setVisible(false);

    if (success) {
        statusLabel->setVisible(false);  // Hide the label on success

        tableView = new OlympicTableView(this);
        mainLayout->addWidget(tableView);

        graphView = new OlympicGraphView(this);
        mainLayout->addWidget(graphView);

        switchView(TableMode);
    } else {
        statusLabel->setText("Error loading data!");
        qDebug() << "Failed to load the CSV file from datasets folder";
    }
}

void MainWindow::updateProgress(int progress) {
    progressBar->setValue(progress);
}

void MainWindow::switchView(ViewMode mode) {
    currentViewMode = mode;

    if (tableView && graphView) {
        switch (mode) {
            case TableMode:
                tableView->setVisible(true);
                graphView->setVisible(false);
                tableViewAction->setChecked(true);
                graphViewAction->setChecked(false);
                graphModeMenu->setEnabled(false);
                break;

            case GraphMode:
                tableView->setVisible(false);
                graphView->setVisible(true);
                tableViewAction->setChecked(false);
                graphViewAction->setChecked(true);
                graphModeMenu->setEnabled(true);
                break;
        }
    }
}

void MainWindow::onGraphModeChanged(int mode)
{
    if (graphView) {
        graphView->setGraphMode(static_cast<OlympicGraphView::GraphMode>(mode));
    }
}

void MainWindow::generateCombinedReport()
{
    if (!tableView || !graphView) {
        QMessageBox::warning(this, tr("Report Generation"),
                             tr("Data needs to be loaded first."));
        return;
    }

    // Capturar o gráfico atual
    QChart* currentChart = nullptr;
    if (graphView) {
        currentChart = graphView->findChild<QChartView*>()->chart();
    }

    // Capturar a tabela atual
    QTableView* currentTable = nullptr;
    if (tableView) {
        currentTable = tableView->findChild<QTableView*>();
    }

    ReportDialog dialog(this, currentChart, currentTable);
    dialog.exec();
}
