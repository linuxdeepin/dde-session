#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDBusInterface>
#include <QDBusServiceWatcher>

#include "sessionadaptor.h"

#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("dde-session-ctl");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption systemd(QStringList{"d", "systemd-service", "wait for systemd services"});
    parser.addOption(systemd);

    parser.process(app);

    QDBusInterface systemdDBus("org.freedesktop.systemd1", "/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager");

    if (parser.isSet(systemd)) {
        QDBusServiceWatcher *watcher = new QDBusServiceWatcher("org.deepin.Session", QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForUnregistration);
        watcher->connect(watcher, &QDBusServiceWatcher::serviceUnregistered, [=] {
            QDBusInterface systemdDBus("org.freedesktop.systemd1", "/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager");
            qInfo() << systemdDBus.call("StartUnit", "dde-session-shutdown.service", "replace");
            qApp->quit();
        });
        qInfo() << systemdDBus.call("StartUnit", "dde-session-x11.target", "replace");
    }
    else {
        auto* session = new Session;
        new SessionAdaptor(session);

        QDBusConnection::sessionBus().registerService("org.deepin.Session");
        QDBusConnection::sessionBus().registerObject("/org/deepin/Session", "org.deepin.Session", session);

        qInfo() << systemdDBus.call("StartUnit", "org.deepin.Session.service", "replace");
    }

    return app.exec();
}
