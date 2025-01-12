#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QDebug>

#include "controller.h"
#include "olympictableview.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    Controller* controller;
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    QLabel* statusLabel;
    QProgressBar* progressBar;
    OlympicTableView* tableView;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleDataLoaded(bool success);
    void updateProgress(int progress);
};

#endif // MAINWINDOW_H
