#include "reportdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDateTime>
#include <QBuffer>
#include <QPainter>

ReportDialog::ReportDialog(QWidget *parent, QChart *chart, QTableView *tableView)
    : QDialog(parent)
    , reportChart(chart)
    , reportTableView(tableView)
{
    setWindowTitle(tr("Generate Custom Report"));
    setMinimumWidth(500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *contentGroup = new QGroupBox(tr("Report Content"));
    QFormLayout *contentLayout = new QFormLayout(contentGroup);

    titleEdit = new QLineEdit(this);
    titleEdit->setText(tr("Olympic Data Analysis Report"));
    contentLayout->addRow(tr("Title:"), titleEdit);

    descriptionEdit = new QTextEdit(this);
    descriptionEdit->setPlaceholderText(tr("Enter a description for this report..."));
    if (reportChart && !reportTableView) {
        descriptionEdit->setText(tr("This report contains graphical analysis of Olympic data."));
    } else if (!reportChart && reportTableView) {
        descriptionEdit->setText(tr("This report contains tabular Olympic data."));
    } else if (reportChart && reportTableView) {
        descriptionEdit->setText(tr("This report contains both graphical and tabular Olympic data."));
    }
    contentLayout->addRow(tr("Description:"), descriptionEdit);

    includeChartCheck = new QCheckBox(tr("Include chart visualization"), this);
    includeChartCheck->setChecked(reportChart != nullptr);
    includeChartCheck->setEnabled(reportChart != nullptr);
    contentLayout->addRow("", includeChartCheck);

    includeTableCheck = new QCheckBox(tr("Include data table"), this);
    includeTableCheck->setChecked(reportTableView != nullptr);
    includeTableCheck->setEnabled(reportTableView != nullptr);
    contentLayout->addRow("", includeTableCheck);

    mainLayout->addWidget(contentGroup);

    QGroupBox *formatGroup = new QGroupBox(tr("Output Options"));
    QFormLayout *formatLayout = new QFormLayout(formatGroup);

    formatCombo = new QComboBox(this);
    formatCombo->addItems({"PDF", "HTML"});
    formatLayout->addRow(tr("Format:"), formatCombo);

    paperSizeCombo = new QComboBox(this);
    paperSizeCombo->addItems({"A4", "Letter", "Legal", "A3"});
    formatLayout->addRow(tr("Paper Size:"), paperSizeCombo);

    orientationCombo = new QComboBox(this);
    orientationCombo->addItems({"Portrait", "Landscape"});
    formatLayout->addRow(tr("Orientation:"), orientationCombo);

    mainLayout->addWidget(formatGroup);

    QGroupBox *metaGroup = new QGroupBox(tr("Metadata"));
    QFormLayout *metaLayout = new QFormLayout(metaGroup);

    authorEdit = new QLineEdit(this);
    authorEdit->setText("Renan Cezar Girardin Pimentel Pontara");
    metaLayout->addRow(tr("Author:"), authorEdit);

    dateEdit = new QDateEdit(QDate::currentDate(), this);
    metaLayout->addRow(tr("Date:"), dateEdit);

    includeHeaderCheck = new QCheckBox(tr("Include header with title and date"), this);
    includeHeaderCheck->setChecked(true);
    metaLayout->addRow("", includeHeaderCheck);

    includeFooterCheck = new QCheckBox(tr("Include footer with page numbers"), this);
    includeFooterCheck->setChecked(true);
    metaLayout->addRow("", includeFooterCheck);

    timestampCheck = new QCheckBox(tr("Include timestamp"), this);
    timestampCheck->setChecked(true);
    metaLayout->addRow("", timestampCheck);

    QHBoxLayout *logoLayout = new QHBoxLayout();
    logoPathEdit = new QLineEdit(this);
    logoPathEdit->setReadOnly(true);
    logoPathEdit->setPlaceholderText(tr("No logo selected"));
    logoButton = new QPushButton(tr("Browse..."), this);
    logoLayout->addWidget(logoPathEdit);
    logoLayout->addWidget(logoButton);
    metaLayout->addRow(tr("Logo:"), logoLayout);

    mainLayout->addWidget(metaGroup);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ReportDialog::generateReport);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ReportDialog::reject);
    connect(logoButton, &QPushButton::clicked, this, &ReportDialog::addLogo);

    connect(formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ReportDialog::updatePreview);
}

void ReportDialog::generateReport()
{
    QString format = formatCombo->currentText().toLower();

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save Report"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
            + QDir::separator() + "Olympic_Report",
        format == "pdf" ? tr("PDF Files (*.pdf)") : tr("HTML Files (*.html)")
        );

    if (fileName.isEmpty()) {
        return;
    }

    if (format == "pdf" && !fileName.endsWith(".pdf", Qt::CaseInsensitive)) {
        fileName += ".pdf";
    } else if (format == "html" && !fileName.endsWith(".html", Qt::CaseInsensitive)) {
        fileName += ".html";
    }

    QString html = "<!DOCTYPE html><html><head>";
    html += "<meta charset=\"UTF-8\">";
    html += "<title>" + titleEdit->text() + "</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 20px; }";
    html += "h1 { color: #0066cc; }";
    html += "table { border-collapse: collapse; width: 100%; margin-top: 20px; }";
    html += "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }";
    html += "th { background-color: #f2f2f2; }";
    html += "tr:nth-child(even) { background-color: #f9f9f9; }";
    html += ".header { background-color: #f0f0f0; padding: 10px; margin-bottom: 20px; }";
    html += ".footer { text-align: center; margin-top: 30px; font-size: 0.8em; color: #666; border-top: 1px solid #ccc; padding-top: 10px; }";
    html += ".logo { float: right; max-height: 100px; margin-left: 20px; }";
    html += ".description { margin: 20px 0; line-height: 1.5; }";
    html += "</style>";
    html += "</head><body>";

    if (includeHeaderCheck->isChecked()) {
        html += "<div class='header'>";
        if (!logoPath.isEmpty()) {
            QFile file(logoPath);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray imageData = file.readAll();
                QString base64 = imageData.toBase64();

                QString extension = logoPath.mid(logoPath.lastIndexOf(".") + 1).toLower();
                html += "<img src='data:image/" + extension + ";base64," + base64 + "' class='logo' alt='Logo'>";
                file.close();
            }
        }
        html += "<h1>" + titleEdit->text() + "</h1>";
        if (timestampCheck->isChecked()) {
            html += "<p>Date: " + dateEdit->date().toString("dd/MM/yyyy") + "</p>";
            html += "<p>Generated: " + QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss") + "</p>";
        }
        html += "<p>Author: " + authorEdit->text() + "</p>";
        html += "</div>";
    }

    html += "<div class='description'>" + descriptionEdit->toPlainText() + "</div>";

    if (includeChartCheck->isChecked() && reportChart) {
        QRectF chartRect = reportChart->geometry();

        int width = std::max(800, static_cast<int>(chartRect.width() * 1.5));
        int height = std::max(600, static_cast<int>(chartRect.height() * 1.5));

        QPixmap pixmap(width, height);
        pixmap.fill(Qt::white);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.setWindow(chartRect.toRect());
        painter.setViewport(QRect(
            width * 0.1,
            height * 0.1,
            width * 0.8,
            height * 0.8
            ));

        reportChart->scene()->render(&painter, QRectF(), chartRect);
        painter.end();

        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");
        buffer.close();

        html += "<div style='text-align:center; margin: 20px 0;'>";
        html += "<img src='data:image/png;base64," + byteArray.toBase64() + "' style='max-width: 100%;'/>";
        html += "</div>";
    }

    if (includeTableCheck->isChecked() && reportTableView) {
        html += "<h2>Data Table</h2>";
        html += generateHTMLTable(reportTableView);
    }

    if (includeFooterCheck->isChecked()) {
        html += "<div class='footer'>";
        html += "<p>Exploração Interativa para a análise de dados de 120 Anos de História Olímpica</p>";
        html += "<p>© " + QString::number(QDate::currentDate().year()) + " " + authorEdit->text() + "</p>";
                html += "</div>";
    }

    html += "</body></html>";

    if (format == "html") {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << html;
            file.close();

            QMessageBox::information(this, tr("Report Generated"),
                                     tr("HTML report has been saved to:\n%1").arg(fileName));
            accept();
        } else {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Failed to save the HTML report."));
        }
    } else if (format == "pdf") {
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);

        QPageSize::PageSizeId paperSize = QPageSize::A4;
        if (paperSizeCombo->currentText() == "Letter") {
            paperSize = QPageSize::Letter;
        } else if (paperSizeCombo->currentText() == "Legal") {
            paperSize = QPageSize::Legal;
        } else if (paperSizeCombo->currentText() == "A3") {
            paperSize = QPageSize::A3;
        }

        QPageLayout::Orientation orientation = QPageLayout::Portrait;
        if (orientationCombo->currentText() == "Landscape") {
            orientation = QPageLayout::Landscape;
        }

        printer.setPageSize(QPageSize(paperSize));
        printer.setPageOrientation(orientation);

        QTextDocument document;
        document.setHtml(html);
        document.print(&printer);

        QMessageBox::information(this, tr("Report Generated"),
                                 tr("PDF report has been saved to:\n%1").arg(fileName));
        accept();
    }
}

void ReportDialog::updatePreview()
{
    // Aqui poderíamos adicionar uma prévia do relatório conforme o formato muda
    // Isso exigiria uma implementação mais complexa com um widget de prévia
}

void ReportDialog::addLogo()
{
    QString filter = tr("Image Files (*.png *.jpg *.jpeg *.bmp *.gif)");
    QString path = QFileDialog::getOpenFileName(
        this,
        tr("Select Logo"),
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        filter
        );

    if (!path.isEmpty()) {
        logoPath = path;
        logoPathEdit->setText(QFileInfo(path).fileName());
    }
}

QString ReportDialog::generateHTMLTable(QTableView* tableView)
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
