#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    controller(new Controller(this)),
    currentViewMode(TableMode)
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

    resize(1600, 1200);

    QString filePath = QDir::currentPath() + "/datasets/athlete_events.csv";
    controller->loadCSV(filePath);
}

MainWindow::~MainWindow()
{
}

void MainWindow::handleDataLoaded(bool success)
{
    progressBar->setVisible(false);

    if (success) {
        statusLabel->setVisible(false);

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

void MainWindow::updateProgress(int progress)
{
    progressBar->setValue(progress);
}

void MainWindow::switchView(ViewMode mode)
{
    currentViewMode = mode;

    if (tableView && graphView) {
        switch (mode) {
        case TableMode:
            tableView->setVisible(true);
            graphView->setVisible(false);
            tableViewAction->setChecked(true);
            graphViewAction->setChecked(false);
            break;

        case GraphMode:
            tableView->setVisible(false);
            graphView->setVisible(true);
            tableViewAction->setChecked(false);
            graphViewAction->setChecked(true);
            break;
        }
    }
}

void MainWindow::setupMenu()
{
    menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    viewMenu = menuBar->addMenu("Modo de Visualização");

    tableViewAction = viewMenu->addAction("Tabela");
    graphViewAction = viewMenu->addAction("Gráfico de Medalhas");

    tableViewAction->setCheckable(true);
    graphViewAction->setCheckable(true);
    tableViewAction->setChecked(true);

    connect(tableViewAction, &QAction::triggered, this, [this]() { switchView(TableMode); });
    connect(graphViewAction, &QAction::triggered, this, [this]() { switchView(GraphMode); });
}
