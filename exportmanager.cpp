#include "exportmanager.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QChartView>
#include <QBuffer>

ExportManager::ExportManager(QObject *parent)
    : QObject(parent)
{
}

bool ExportManager::exportChart(QChart* chart, const QString& fileName, const QString& format)
{
    if (!chart) {
        return false;
    }

    if (format.toLower() == "pdf") {
        return exportChartToPDF(chart, fileName);
    } else if (format.toLower() == "svg") {
        return exportChartToSVG(chart, fileName);
    } else {
        return exportChartToImage(chart, fileName, format);
    }
}

bool ExportManager::exportFilteredData(QTableView* tableView, const QString& fileName, const QString& format)
{
    if (!tableView) {
        return false;
    }

    if (format.toLower() == "csv") {
        return exportToCSV(tableView, fileName);
    } else if (format.toLower() == "xlsx" || format.toLower() == "xls") {
        return exportToExcel(tableView, fileName);
    } else if (format.toLower() == "json") {
        return exportToJSON(tableView, fileName);
    } else if (format.toLower() == "html") {
        return exportToHTML(tableView, fileName);
    } else if (format.toLower() == "pdf") {
        return exportToPDF(tableView, fileName);
    }

    return false;
}

bool ExportManager::generateReport(QWidget* parent, const QString& title, const QString& description,
                                 QChart* chart, QTableView* tableView)
{
    QString fileName = QFileDialog::getSaveFileName(parent,
                                                  tr("Salvar Relatório"),
                                                  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                  tr("PDF Files (*.pdf)"));
    if (fileName.isEmpty()) {
        return false;
    }

    if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) {
        fileName += ".pdf";
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));

    QTextDocument document;

    QString html = "<html><head><style>"
                  "body { font-family: Arial, sans-serif; }"
                  "h1 { color: #003366; }"
                  "h2 { color: #0066cc; }"
                  ".header { background-color: #f0f0f0; padding: 10px; }"
                  ".content { padding: 10px; }"
                  "table { border-collapse: collapse; width: 100%; }"
                  "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }"
                  "th { background-color: #f2f2f2; }"
                  "tr:nth-child(even) { background-color: #f9f9f9; }"
                  ".footer { text-align: center; margin-top: 20px; font-size: 0.8em; color: #666; }"
                  "</style></head><body>";

    QDateTime currentDateTime = QDateTime::currentDateTime();
    html += "<div class='header'>";
    html += "<h1>" + title + "</h1>";
    html += "<p>Gerado em: " + currentDateTime.toString("dd/MM/yyyy HH:mm:ss") + "</p>";
    html += "</div>";

    html += "<div class='content'>";
    html += "<p>" + description + "</p>";

    if (chart) {
        QChartView tempChartView(chart);
        tempChartView.resize(chart->size().toSize());
        tempChartView.setRenderHint(QPainter::Antialiasing);

        QPixmap pixmap(chart->size().toSize());
        pixmap.fill(Qt::white);
        QPainter painter(&pixmap);
        tempChartView.render(&painter);

        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");
        buffer.close();

        html += "<h2>Visualização Gráfica</h2>";
        html += "<img src='data:image/png;base64," + byteArray.toBase64() + "' style='max-width: 100%;'/>";
    }

    if (tableView) {
        html += "<h2>Dados</h2>";
        html += generateHTMLTable(tableView);
    }

    html += "<div class='footer'>";
    html += "<p>Exploração Interativa para a análise de dados de 120 Anos de História Olímpica</p>";
    html += "<p>© " + QString::number(currentDateTime.date().year()) + " Renan Cezar Girardin Pimentel Pontara</p>";
    html += "</div>";

    html += "</div></body></html>";

    document.setHtml(html);
    document.print(&printer);

    QMessageBox::information(parent, tr("Relatório Gerado"),
                           tr("Relatório salvo com sucesso em:\n%1").arg(fileName));

    return true;
}

bool ExportManager::exportToCSV(QTableView* tableView, const QString& fileName)
{
    QAbstractItemModel* model = tableView->model();
    if (!model) {
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);

    QStringList headers;
    for (int i = 0; i < model->columnCount(); ++i) {
        headers << model->headerData(i, Qt::Horizontal).toString();
    }
    out << headers.join(",") << "\n";

    for (int row = 0; row < model->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < model->columnCount(); ++col) {
            QModelIndex index = model->index(row, col);
            QString data = model->data(index).toString();
            if (data.contains(",") || data.contains("\"") || data.contains("\n")) {
                data.replace("\"", "\"\"");
                data = "\"" + data + "\"";
            }
            rowData << data;
        }
        out << rowData.join(",") << "\n";
    }

    file.close();
    return true;
}

bool ExportManager::exportToExcel(QTableView* tableView, const QString& fileName)
{
    QString csvFileName = fileName;
    if (csvFileName.endsWith(".xlsx", Qt::CaseInsensitive) ||
        csvFileName.endsWith(".xls", Qt::CaseInsensitive)) {
        csvFileName = csvFileName.left(csvFileName.lastIndexOf(".")) + ".csv";
    }

    bool success = exportToCSV(tableView, csvFileName);

    if (success) {
        QMessageBox::information(nullptr, tr("Exportação Concluída"),
                               tr("Os dados foram exportados para CSV em:\n%1\n\nObservação: Para Excel completo, "
                                  "abra este arquivo no Excel e salve como XLSX.").arg(csvFileName));
    }

    return success;
}

bool ExportManager::exportToJSON(QTableView* tableView, const QString& fileName)
{
    QAbstractItemModel* model = tableView->model();
    if (!model) {
        return false;
    }

    QJsonArray jsonArray;

    QStringList headers;
    for (int i = 0; i < model->columnCount(); ++i) {
        headers << model->headerData(i, Qt::Horizontal).toString();
    }

    for (int row = 0; row < model->rowCount(); ++row) {
        QJsonObject jsonObj;
        for (int col = 0; col < model->columnCount(); ++col) {
            QModelIndex index = model->index(row, col);
            QVariant value = model->data(index);

            switch (value.typeId()) {
                case QMetaType::Int:
                case QMetaType::UInt:
                case QMetaType::LongLong:
                case QMetaType::ULongLong:
                    jsonObj[headers[col]] = value.toInt();
                    break;
                case QMetaType::Double:
                    jsonObj[headers[col]] = value.toDouble();
                    break;
                case QMetaType::Bool:
                    jsonObj[headers[col]] = value.toBool();
                    break;
                default:
                    jsonObj[headers[col]] = value.toString();
            }
        }
        jsonArray.append(jsonObj);
    }

    QJsonDocument jsonDoc(jsonArray);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(jsonDoc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

bool ExportManager::exportToHTML(QTableView* tableView, const QString& fileName)
{
    QString htmlContent = "<html><head>"
                        "<meta charset=\"UTF-8\">"
                        "<title>Dados Olímpicos Exportados</title>"
                        "<style>"
                        "body { font-family: Arial, sans-serif; margin: 20px; }"
                        "h1 { color: #0066cc; }"
                        "table { border-collapse: collapse; width: 100%; }"
                        "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }"
                        "th { background-color: #f2f2f2; }"
                        "tr:nth-child(even) { background-color: #f9f9f9; }"
                        ".footer { margin-top: 20px; font-size: 0.8em; color: #666; }"
                        "</style>"
                        "</head><body>"
                        "<h1>Dados Olímpicos Exportados</h1>"
                        "<p>Exportado em: " + QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss") + "</p>";

    htmlContent += generateHTMLTable(tableView);

    htmlContent += "<div class=\"footer\">"
                  "<p>Gerado pelo OlympicBrowser - Análise de dados de 120 Anos de História Olímpica</p>"
                  "</div>"
                  "</body></html>";

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << htmlContent;

    file.close();
    return true;
}

bool ExportManager::exportToPDF(QTableView* tableView, const QString& fileName)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));

    QTextDocument document;
    QString htmlContent = "<html><head>"
                        "<style>"
                        "body { font-family: Arial, sans-serif; }"
                        "h1 { color: #0066cc; }"
                        "table { border-collapse: collapse; width: 100%; }"
                        "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }"
                        "th { background-color: #f2f2f2; }"
                        "tr:nth-child(even) { background-color: #f9f9f9; }"
                        ".footer { text-align: center; margin-top: 20px; font-size: 0.8em; color: #666; }"
                        "</style>"
                        "</head><body>"
                        "<h1>Dados Olímpicos</h1>"
                        "<p>Exportado em: " + QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss") + "</p>";

    htmlContent += generateHTMLTable(tableView);

    htmlContent += "<div class=\"footer\">"
                  "<p>Exploração Interativa para a análise de dados de 120 Anos de História Olímpica</p>"
                  "</div>"
                  "</body></html>";

    document.setHtml(htmlContent);
    document.print(&printer);

    return true;
}

bool ExportManager::exportChartToImage(QChart* chart, const QString& fileName, const QString& format)
{
    if (!chart) {
        return false;
    }

    QChartView tempChartView(chart);
    tempChartView.resize(chart->size().toSize());
    tempChartView.setRenderHint(QPainter::Antialiasing);

    QPixmap pixmap(chart->size().toSize());
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    tempChartView.render(&painter);

    return pixmap.save(fileName, format.toUpper().toUtf8().constData());
}

bool ExportManager::exportChartToPDF(QChart* chart, const QString& fileName)
{
    if (!chart) {
        return false;
    }

    QChartView tempChartView(chart);
    tempChartView.resize(chart->size().toSize());
    tempChartView.setRenderHint(QPainter::Antialiasing);

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);

    QPainter painter(&printer);
    tempChartView.render(&painter);
    painter.end();

    return true;
}

bool ExportManager::exportChartToSVG(QChart* chart, const QString& fileName)
{
    if (!chart) {
        return false;
    }

    QChartView tempChartView(chart);
    tempChartView.resize(chart->size().toSize());
    tempChartView.setRenderHint(QPainter::Antialiasing);

    QSvgGenerator generator;
    generator.setFileName(fileName);
    generator.setSize(chart->size().toSize());
    generator.setViewBox(QRect(0, 0, chart->size().width(), chart->size().height()));
    generator.setTitle("Olympic Data Chart");
    generator.setDescription("Chart generated by Olympic Browser");

    QPainter painter(&generator);
    tempChartView.render(&painter);
    painter.end();

    return true;
}

QString ExportManager::generateHTMLTable(QTableView* tableView)
{
    QAbstractItemModel* model = tableView->model();
    if (!model) {
        return "";
    }

    QString tableHtml = "<table><thead><tr>";

    for (int col = 0; col < model->columnCount(); ++col) {
        tableHtml += "<th>" + model->headerData(col, Qt::Horizontal).toString() + "</th>";
    }

    tableHtml += "</tr></thead><tbody>";

    for (int row = 0; row < model->rowCount(); ++row) {
        tableHtml += "<tr>";
        for (int col = 0; col < model->columnCount(); ++col) {
            QModelIndex index = model->index(row, col);
            tableHtml += "<td>" + model->data(index).toString() + "</td>";
        }
        tableHtml += "</tr>";
    }

    tableHtml += "</tbody></table>";

    return tableHtml;
}
