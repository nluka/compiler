#include "CompilerTestsWindow.hpp"
#include "CompilationFlowWindow.hpp"

#include "populateCommonMenuBar.hpp"

void populateCommonMenuBar(QMenuBar *menu_bar)
{
    {
        QMenu *window_menu = menu_bar->addMenu("&Window");

        QAction *empty_compilation_flow_action = new QAction("New &Compilation Flow", menu_bar);

        window_menu->addAction(empty_compilation_flow_action);

        QObject::connect(empty_compilation_flow_action, &QAction::triggered, menu_bar, []() {
            QString title = "Compilation Flow";
            CompilationFlowWindow *w = new CompilationFlowWindow(nullptr, title);
            w->setAttribute(Qt::WA_DeleteOnClose);
            w->resize(1600, 900);
            w->show();
        });
    }
    {
        QMenu *debug_menu = menu_bar->addMenu("&Debug");

        QAction *ASan_action = new QAction("&Trigger ASan", menu_bar);

        debug_menu->addAction(ASan_action);

        QObject::connect(ASan_action, &QAction::triggered, menu_bar, []() {
            int *p = new int[4];
            p[4] = 123; // intentional heap buffer overflow
        });
    }
}