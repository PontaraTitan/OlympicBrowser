#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QDateEdit>
#include <QChart>
#include <QChartView>
#include <QTableView>
#include <QFileDialog>
#include <QMessageBox>
#include <QBuffer>
#include <QPrinter>
#include <QPainter>
#include <QGraphicsScene>

class ReportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ReportDialog(QWidget *parent = nullptr, QChart *chart = nullptr, QTableView *tableView = nullptr);

private slots:
    void generateReport();
    void updatePreview();
    void addLogo();

private:
    // Função auxiliar para gerar uma tabela HTML a partir de um QTableView
    QString generateHTMLTable(QTableView* tableView);

    QLineEdit *titleEdit;
    QTextEdit *descriptionEdit;
    QComboBox *formatCombo;
    QCheckBox *includeChartCheck;
    QCheckBox *includeTableCheck;
    QCheckBox *includeHeaderCheck;
    QCheckBox *includeFooterCheck;
    QLineEdit *authorEdit;
    QComboBox *paperSizeCombo;
    QComboBox *orientationCombo;
    QDateEdit *dateEdit;
    QLineEdit *logoPathEdit;
    QPushButton *logoButton;
    QCheckBox *timestampCheck;
    QDialogButtonBox *buttonBox;

    QChart *reportChart;
    QTableView *reportTableView;

    QString logoPath;
};

#endif // REPORTDIALOG_H
