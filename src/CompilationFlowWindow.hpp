#ifndef COMPILATIONFLOWWINDOW_HPP
#define COMPILATIONFLOWWINDOW_HPP

#include <QWidget>

class CompilationFlowWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CompilationFlowWindow(QWidget *parent = nullptr, QString const &title = "Compilation Flow");
};

#endif // COMPILATIONFLOWWINDOW_HPP
