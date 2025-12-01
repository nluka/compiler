#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>

class DirectoryPicker : public QWidget {
    Q_OBJECT

public:
    explicit DirectoryPicker(char const *label,
                             char const *initial_value = "",
                             QWidget *parent = nullptr);

    QString path() const;

signals:
    void pathChanged(QString const &path);

public slots:
    void setPath(QString const &path);

private slots:
    void onBrowse();

private:
    QLineEdit *pathEdit;
};
