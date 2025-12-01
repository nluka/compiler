#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>

class FilePicker : public QWidget {
    Q_OBJECT

public:
    explicit FilePicker(char const *label,
                        char const *initial_value = "",
                        QWidget *parent = nullptr);

    QString file() const;

public slots:
    void setFile(QString const &path);

signals:
    void fileChanged(QString const &path);

private slots:
    void onBrowse();

private:
    QLineEdit *fileEdit;
};
