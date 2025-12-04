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
#include <QMenuBar>

#include "util.hpp"

#include "MainWindow.hpp"

using CsvTable = QVector<QVector<QString>>;

void MainWindow::loadCsv(QString const &csvPath)
{
    QFile file(csvPath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QTextStream in(&file);

    if (in.atEnd())
        return; // Empty file

    QString const headerLine = in.readLine().trimmed();
    QStringList const headers = headerLine.split(',');

    static QStringList const required = {
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

    int const statusCol = 0;
    int const compilationFlowCol = 1;
    int const firstCsvCol = 2; // Table column where actual CSV fields begin
    int const requiredCsvColumnCount = required.size();
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
        tableHeaders << "Status" << "Compilation Flow";
        tableHeaders.append(required); // Append the CSV columns
        testsTable->setHorizontalHeaderLabels(tableHeaders);
    }

    // Preserve "Status" from previous state (testRows) by computing a hash for the fields and checking if that row existed in previous state.
    QVector<RowState> newRows;

    auto hashRow = [](QStringList const &fields) -> QByteArray {
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
            testsTable->setCellWidget(r, compilationFlowCol, btn);

            connect(btn, &QPushButton::clicked, this, [this, fields]() {
                QString title = "Compilation Flow (" + fields[0] + ")";
                CompilationFlowWindow *flow = new CompilationFlowWindow(nullptr, title);
                flow->resize(1600, 900);
                flow->show();
            });
        }
        for (int c = 0; c < requiredCsvColumnCount; ++c) {
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

void MainWindow::onCsvFileModified(QString const &path)
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
    QString const &value = item->text();

    qDebug() << "Cell changed: row" << row << "col" << col << "->" << value;

    int statusCol = 0;
    if (col == statusCol) {
        testRows[row].status = value;
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    {
        QMenuBar *menu_bar = this->menuBar();
        {
            QMenu *window_menu = menu_bar->addMenu("&Window");
            QAction *empty_compilation_flow_action = new QAction("New &Compilation Flow", this);
            window_menu->addAction(empty_compilation_flow_action);
            QObject::connect(empty_compilation_flow_action, &QAction::triggered, this, []() {
                QString title = "Compilation Flow";
                CompilationFlowWindow *flow = new CompilationFlowWindow(nullptr, title);
                flow->resize(1600, 900);
                flow->show();
            });
        }
        {
            QMenu *debug_menu = menu_bar->addMenu("&Debug");
            QAction *ASan_action = new QAction("&Trigger ASan", this);
            debug_menu->addAction(ASan_action);
            QObject::connect(ASan_action, &QAction::triggered, this, []() {
                int *p = new int[4];
                p[4] = 123; // intentional heap buffer overflow
            });
        }
    }

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

    testsTable = new QTableWidget(this);
    loadCsv(csvFilePicker->file());
    testsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    testsTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    top_hbox->addWidget(csvFilePicker);
    top_hbox->addWidget(dataDirectoryPicker);

    main_vbox->addLayout(top_hbox);
    main_vbox->addWidget(testsTable);

    setCentralWidget(central); // Required for QMainWindow
    resize(900, 600);
}
