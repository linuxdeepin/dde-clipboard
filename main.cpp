#include "mainwindow.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();
    DApplication app(argc, argv);

    app.setOrganizationName("deepin");
    app.setApplicationName("dde-clipboard");
    app.setApplicationDisplayName("DDE Clipboard");
    app.setApplicationVersion("1.0");
    app.loadTranslator();

    MainWindow w;
    w.show();

    return app.exec();
}
