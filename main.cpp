#include "mainwindow.h"
#include "constants.h"
#include "clipboard_adaptor.h"

#include <DApplication>
#include <DGuiApplicationHelper>
#include <DLog>
#include <unistd.h>

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

    if (!DGuiApplicationHelper::setSingleInstance(QString("dde-clipboard_%1").arg(getuid()))) {
        qDebug() << "set single instance failed!";
        return -1;
    }

    QDBusConnection connection = QDBusConnection::sessionBus();

    MainWindow w;
    ClipboardAdaptor adapt(&w);
    if (!connection.registerService(DBusClipBoardService)) {
        qDebug() << "error:" << connection.lastError().message();
        exit(-1);
    }

    connection.registerObject(DBusClipBoardPath, &w);

    w.show();

    return app.exec();
}
