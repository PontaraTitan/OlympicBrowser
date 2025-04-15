#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QDebug>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>

#include "controller.h"
#include "olympictableview.h"
#include "olympicgraphview.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    enum ViewMode {
        TableMode,
        GraphMode
    };

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleDataLoaded(bool success);
    void updateProgress(int progress);
    void switchView(ViewMode mode);
    void onGraphModeChanged(int mode);
    void generateCombinedReport();

private:
    void setupMenu();

private:
    Controller* controller;
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    QLabel* statusLabel;
    QProgressBar* progressBar;
    OlympicTableView* tableView;
    OlympicGraphView* graphView;

    QMenuBar* menuBar;
    QMenu* viewMenu;
    QMenu* graphModeMenu;
    QAction* tableViewAction;
    QAction* graphViewAction;
    QAction* medalEvolutionAction;
    QAction* demographicsAction;
    QAction* countryComparisonAction;
    QAction* geographicAction;
    QAction* statisticalAction;
    QAction* combinedReportAction;

    ViewMode currentViewMode;
};

#endif // MAINWINDOW_H
