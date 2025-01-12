#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , controller(new Controller(this))
{
    // Setup UI
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);

    statusLabel = new QLabel("Loading data...", this);
    progressBar = new QProgressBar(this);

    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(progressBar);

    progressBar->setVisible(true);

    // Connect signals/slots
    connect(controller, &Controller::dataLoaded, this, &MainWindow::handleDataLoaded);
    connect(controller, &Controller::progressUpdated, this, &MainWindow::updateProgress);

    resize(800, 600);

    // Load data automatically
    QString filePath = QDir::currentPath() + "/datasets/athlete_events.csv";
    controller->loadCSV(filePath);
}

MainWindow::~MainWindow() {
}

void MainWindow::handleDataLoaded(bool success) {
    progressBar->setVisible(false);

    if (success) {
        statusLabel->setVisible(false);  // Hide the label on success
        tableView = new OlympicTableView(this);
        mainLayout->addWidget(tableView);
    } else {
        statusLabel->setText("Error loading data!");
        qDebug() << "Failed to load the CSV file from datasets folder";
    }
}

void MainWindow::updateProgress(int progress) {
    progressBar->setValue(progress);
}
