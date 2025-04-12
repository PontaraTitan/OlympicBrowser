#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDebug>

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
    QAction* tableViewAction;
    QAction* graphViewAction;

    ViewMode currentViewMode;
};

#endif // MAINWINDOW_H
