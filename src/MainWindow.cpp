#include <QDebug>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QFileDialog>
#include <QPushButton>
#include <QVector>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QHeaderView>
#include <QStringList>
#include <QMessageBox>
#include <QCryptographicHash>

#include "util.hpp"

#include "MainWindow.hpp"
#include "ui_MainWindow.h"

using CsvTable = QVector<QVector<QString>>;

#if 0
CsvTable loadCsvFile(const QString &filePath)
{
    CsvTable table;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << filePath;
        return table;
    }

    QTextStream stream(&file);

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QVector<QString> row;

        QString field;
        bool inQuotes = false;

        for (int i = 0; i < line.size(); ++i) {
            QChar c = line[i];

            if (c == '"') {
                if (inQuotes && i + 1 < line.size() && line[i+1] == '"') {
                    field += '"';   // escaped quote
                    ++i;
                } else {
                    inQuotes = !inQuotes;
                }
            }
            else if (c == ',' && !inQuotes) {
                row.push_back(field);
                field.clear();
            }
            else {
                field += c;
            }
        }

        row.push_back(field);
        table.push_back(row);
    }

    return table;
}

void populateTableWidget(QMainWindow *main_window, QTableWidget *table, const CsvTable &data)
{
    if (data.size() <= 1) // need at least header + one row
    return;

    // The first row is now headers
    const QVector<QString> &headerRow = data[0];
    int originalCols = headerRow.size();
    int totalCols = originalCols + 2; // "Pass" + "Open"

    int dataRows = data.size() - 1; // exclude header row
    table->setColumnCount(totalCols);
    table->setRowCount(dataRows);

    // Set headers
    QStringList headers;
    headers << "Explore" << "Pass"; // extra columns
    for (const QString &h : headerRow)
    headers << h;

    table->setHorizontalHeaderLabels(headers);

    // Populate rows starting from data[1]
    for (int r = 0; r < dataRows; ++r)
    {
        const QVector<QString> &rowData = data[r + 1]; // skip header row

        // "Open" column with button
        QPushButton *openButton = new QPushButton("Open");
        table->setCellWidget(r, 0, openButton);

        // "Pass" column left blank
        table->setItem(r, 1, new QTableWidgetItem(""));

        // Capture row data for the button
        QObject::connect(openButton, &QPushButton::clicked, [rowData, main_window]() {
            QString title = "Compilation Flow (" + rowData[0] + ")";
            CompilationFlowWindow *flow = new CompilationFlowWindow(main_window, title);
            flow->show();
        });

        // Fill remaining columns with CSV data
        for (int c = 0; c < originalCols; ++c)
        {
            table->setItem(r, c + 2, new QTableWidgetItem(rowData[c]));
        }
    }

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}
#endif

void MainWindow::loadCsv(QString const &csvPath)
{
    QFile file(csvPath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QTextStream in(&file);

    if (in.atEnd())
        return; // Empty file

    const QString headerLine = in.readLine().trimmed();
    const QStringList headers = headerLine.split(',');

    static const QStringList required = {
        "Test Name",
        "Source File",
        "Expected Output File"
    };

    if (headers != required) {
        QMessageBox::critical(
            this,
            "CSV Error",
            "The CSV columns do not match the required structure:\n"
            "Test Name, Source File, Expected Output File"
        );
        return;
    }

    const int statusCol = 0;
    const int openCol = 1;
    const int firstCsvCol = 2; // Table column where actual CSV fields begin
    const int requiredCsvColumnCount = required.size();
    QVector<QStringList> newCsvData;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;

        QStringList fields = line.split(',');

        // Enforce exact column count
        if (fields.size() != requiredCsvColumnCount) {
            QMessageBox::critical(
                this,
                "CSV Error",
                QString("Malformed row:\n%1\nExpected %2 columns.")
                    .arg(line)
                    .arg(requiredCsvColumnCount)
            );
            return;
        }

        newCsvData.push_back(fields);
    }

    testsTable->clear();
    testsTable->setRowCount(newCsvData.size());
    testsTable->setColumnCount(firstCsvCol + requiredCsvColumnCount);
    {
        QStringList tableHeaders;
        tableHeaders << "Status" << "Open";
        tableHeaders.append(required); // Append the CSV columns
        testsTable->setHorizontalHeaderLabels(tableHeaders);
    }

    // Preserve "Status" from previous state (testRows) by computing a hash for the fields and checking if that row existed in previous state.
    QVector<RowState> newRows;

    auto hashRow = [](const QStringList &fields) -> QByteArray {
        return QCryptographicHash::hash(
            fields.join("|").toUtf8(),
            QCryptographicHash::Sha256
        );
    };

    QMap<QByteArray /* hash */, RowState> hashToOldState;

    for (int r = 0; r < testRows.size(); ++r) {
        hashToOldState[hashRow(testRows[r].fields)] = testRows[r];
    }

    testsTableIsPopulating = true;
    testsTable->blockSignals(true); // Prevent following loop from triggering itemChanged signal

    for (int r = 0; r < newCsvData.size(); ++r) {
        QStringList const &fields = newCsvData[r];
        QByteArray hash = hashRow(fields);

        QString statusValue = hashToOldState.contains(hash) ? hashToOldState[hash].status : "";

        newRows.append({ statusValue, fields });
        {
            QTableWidgetItem *statusItem = new QTableWidgetItem(statusValue);
            testsTable->setItem(r, statusCol, statusItem);
            // statusItem->setFlags(statusItem->flags() & ~Qt::ItemIsEditable);
        }
        {
            QPushButton *btn = new QPushButton("Open");
            testsTable->setCellWidget(r, openCol, btn);

            connect(btn, &QPushButton::clicked, this, [this, fields]() {
                QString title = "Compilation Flow (" + fields[0] + ")";
                CompilationFlowWindow *flow = new CompilationFlowWindow(nullptr, title);
                flow->show();
            });
        }
        for (int c = 0; c < requiredCsvColumnCount; c++) {
            QTableWidgetItem *item = new QTableWidgetItem(fields[c]);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            testsTable->setItem(r, firstCsvCol + c, item);
        }
    }
    testsTable->blockSignals(false);
    testsTableIsPopulating = false;

    // Listen for when a row gets updated so we can update the RowState (testRows) accordingly
    connect(testsTable, &QTableWidget::itemChanged,
            this, &MainWindow::onTableItemChanged);

    testRows = newRows;
}

void MainWindow::onCsvPathChanged(const QString &path)
{
    qDebug() << "MainWindow::onCsvPathChanged( " << path << " )";

    if (path.isEmpty())
        return;

    watcher->removePaths(watcher->files()); // Stop watching old file
    watcher->addPath(path); // Start watching new file

    loadCsv(path);
}

void MainWindow::onCsvFileModified(const QString &path)
{
    qDebug() << "MainWindow::onCsvFileModified( " << path << " )";

    // Text editors often rewrite the file, so the watcher is lost.
    // Re-add the path to keep watching it.
    if (QFile::exists(path)) {
        watcher->addPath(path);
    }
    loadCsv(path);
}

void MainWindow::onTableItemChanged(QTableWidgetItem *item)
{
    if (testsTableIsPopulating)
        return;

    int row = item->row();
    int col = item->column();
    QString value = item->text();

    qDebug() << "Cell changed: row" << row << "col" << col << "->" << value;

    int statusCol = 0;
    if (col == statusCol) {
        testRows[row].status = value;
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , compilationFlowWindow(nullptr)
{
    // ui->setupUi(this);
    QWidget *central = new QWidget(this);
    QVBoxLayout *main_vbox = new QVBoxLayout(central);
    QHBoxLayout *top_hbox = new QHBoxLayout(central);

    central->setLayout(main_vbox);

    csvFilePicker = new FilePicker("Tests CSV:", "E:/Dev/compiler/tests/compiler.csv", this);
    dataDirectoryPicker = new DirectoryPicker("Tests Data:", "E:/Dev/compiler/tests/data", this);

    watcher = new QFileSystemWatcher(this);
    watcher->addPath(csvFilePicker->file());

    // When the file is modified, reload the CSV
    connect(watcher, &QFileSystemWatcher::fileChanged,
            this, &MainWindow::onCsvFileModified);

    // FilePicker emits fileSelected(QString)
    connect(csvFilePicker, &FilePicker::fileChanged,
            this, &MainWindow::onCsvPathChanged);

    // CsvTable csvTable = loadCsvFile(csv_picker->file());
    testsTable = new QTableWidget(this);
    // populateTableWidget(this, table, csvTable);
    loadCsv(csvFilePicker->file());
    testsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    testsTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    top_hbox->addWidget(csvFilePicker);
    top_hbox->addWidget(dataDirectoryPicker);

    main_vbox->addLayout(top_hbox);
    main_vbox->addWidget(testsTable);

    setCentralWidget(central); // Required for QMainWindow
    resize(900, 600);
    // connect(ui->btnOpenCompilationFlowWindow, &QPushButton::clicked, this, &MainWindow::on_btnOpenCompilationFlowWindow_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnOpenCompilationFlowWindow_clicked()
{
    qDebug() << "CompilationFlowWindow";

    if (!compilationFlowWindow) {
        compilationFlowWindow = new CompilationFlowWindow(nullptr); // parent optional
    }
    compilationFlowWindow->resize(1280, 720);
    compilationFlowWindow->show();
    // compilationFlowWindow->showMaximized();
    // compilationFlowWindow->raise(); // bring to front
    // compilationFlowWindow->activateWindow(); // focus
}

#include "MainWindow.moc"
