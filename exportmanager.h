#ifndef EXPORTMANAGER_H
#define EXPORTMANAGER_H

#include <QObject>
#include <QWidget>
#include <QTableView>
#include <QFileDialog>
#include <QMessageBox>
#include <QPrinter>
#include <QPainter>
#include <QPdfWriter>
#include <QTextDocument>
#include <QSvgGenerator>
#include <QImageWriter>
#include <QChart>

class ExportManager : public QObject {
    Q_OBJECT

public:
    explicit ExportManager(QObject *parent = nullptr);

    // Exportar visualização
    bool exportChart(QChart* chart, const QString& fileName, const QString& format);

    // Exportar dados filtrados
    bool exportFilteredData(QTableView* tableView, const QString& fileName, const QString& format);

    // Gerar relatório
    bool generateReport(QWidget* parent, const QString& title, const QString& description,
                        QChart* chart = nullptr, QTableView* tableView = nullptr);

private:
    bool exportToCSV(QTableView* tableView, const QString& fileName);
    bool exportToExcel(QTableView* tableView, const QString& fileName);
    bool exportToJSON(QTableView* tableView, const QString& fileName);
    bool exportToHTML(QTableView* tableView, const QString& fileName);
    bool exportToPDF(QTableView* tableView, const QString& fileName);

    bool exportChartToImage(QChart* chart, const QString& fileName, const QString& format);
    bool exportChartToPDF(QChart* chart, const QString& fileName);
    bool exportChartToSVG(QChart* chart, const QString& fileName);

    QString generateHTMLTable(QTableView* tableView);
};

#endif // EXPORTMANAGER_H
