#pragma once

#include <QMainWindow>
#include <QVector>
#include <QPushButton>
#include <QWidget>

class StartWindow : public QMainWindow
{
    Q_OBJECT

public:
    StartWindow(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QVector<QPushButton *> m_buttons;

    void openCompilerTests();
    void openCompilationFlow();
    void openDSA();
    void openFileUtil();
    void openKeyCap();
    void openLangton();
    void openPacketCapture();
    void openPerfAware();
    void openSwan();
};
