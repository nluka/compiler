#include "FilePicker.hpp"

FilePicker::FilePicker(char const *label,
                       char const *initial_value,
                       QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    fileEdit = new QLineEdit(this);
    fileEdit->setText(initial_value);

    QPushButton *browseButton = new QPushButton("Browse…", this);
    QLabel *qlabel = new QLabel(label);

    layout->addWidget(qlabel);
    layout->addWidget(fileEdit);
    layout->addWidget(browseButton);

    connect(browseButton, &QPushButton::clicked,
            this, &FilePicker::onBrowse);
}

QString FilePicker::file() const {
    return fileEdit->text();
}

void FilePicker::setFile(QString const &path) {
    fileEdit->setText(path);
    emit fileChanged(path);
}

void FilePicker::onBrowse() {
    QString path = QFileDialog::getOpenFileName(
        this,
        "Select File",
        fileEdit->text()
    );

    if (!path.isEmpty())
        setFile(path);
}
