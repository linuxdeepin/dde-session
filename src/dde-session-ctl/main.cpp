#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDBusInterface>

#include "Session.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("dde-session-ctl");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption shutdown(QStringList{ "s", "shutdown", "shutdown dde"});
    parser.addOption(shutdown);

    QCommandLineOption sessionExit(QStringList{"S", "session-exit", "session exit task"});
    parser.addOption(sessionExit);

    QCommandLineOption logout(QStringList{"l", "logout", "logout session"});
    parser.addOption(logout);

    parser.process(app);

    const bool isShutdown = parser.isSet(shutdown);
    const bool isSessionExit = parser.isSet(sessionExit);

    if (parser.isSet(logout)) {
        org::deepin::dde::Session1 session("org.deepin.dde.Session1", "/org/deepin/dde/Session1", QDBusConnection::sessionBus());
        session.Logout();

        return 0;
    }

    if (isShutdown) {
        // kill startdde-session or call login1
        QDBusInterface systemd("org.freedesktop.systemd1", "/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager");
        qInfo() << systemd.call("StartUnit", "dde-session-shutdown.target", "replace-irreversibly");
        return 0;
    }

    if (isSessionExit) {
        QDBusInterface systemd("org.freedesktop.systemd1", "/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager");
        qInfo() << systemd.call("StopUnit", "dbus.service", "replace-irreversibly");
        return 0;
    }

    return 0;
}
