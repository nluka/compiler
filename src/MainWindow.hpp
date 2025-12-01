#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QFileSystemWatcher>
#include <QTableWidget>

#include "FilePicker.hpp"
#include "DirectoryPicker.hpp"

#include "CompilationFlowWindow.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadCsv(QString const &csvPath);

private slots:
    void on_btnOpenCompilationFlowWindow_clicked();
    void onCsvPathChanged(const QString &path);
    void onCsvFileModified(const QString &path);
    void onTableItemChanged(QTableWidgetItem *item);

private:
    Ui::MainWindow *ui;
    CompilationFlowWindow *compilationFlowWindow; // pointer to second window
    QFileSystemWatcher *watcher;
    FilePicker *csvFilePicker;
    DirectoryPicker *dataDirectoryPicker;

    QTableWidget *testsTable;
    bool testsTableIsPopulating = false;

    struct RowState {
        QString status;
        QStringList fields;
    };
    // QMap<QString, RowState> lastTableState;
    QVector<RowState> testRows;
};

#endif // MAINWINDOW_HPP
