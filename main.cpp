#include "mainwindow.h"
#include <DApplication>
#include <DGuiApplicationHelper>
#include <DLog>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();
    DGuiApplicationHelper::setUseInactiveColorGroup(false);
    DGuiApplicationHelper::setColorCompositingEnabled(true);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

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
