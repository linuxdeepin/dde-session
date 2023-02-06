// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDBusInterface>
#include <QProcess>
#include <QVector>
#include <QDBusObjectPath>
#include <QDBusVariant>

#include "Session.h"

QVector<uint> getStartddePID() 
{
    QProcess process;
    process.setProgram("pidof");
    QStringList argument("startdde");
    process.setArguments(argument);
    process.start();
    process.waitForFinished();
    QStringList pids = QString::fromLocal8Bit(process.readAllStandardOutput()).simplified().split(" ");
    QVector<uint> ret;
    for (auto pid : pids) {
        if (!pid.isEmpty()) {
            ret.append(pid.toUInt());
        }
    }
    qDebug() << "Startdde Pids:" << ret;
    return ret;
}

bool checkStartddeSession()
{
    QVector<uint> pids = getStartddePID();
    for (uint pid : pids) {
        QDBusInterface systemd("org.freedesktop.systemd1", "/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager", QDBusConnection::systemBus());
        QDBusReply<QDBusObjectPath> reply = systemd.call("GetUnitByPID", pid);
        if (!reply.isValid()) {
            continue;
        }
        qInfo() << "startdde unit:" << reply.value().path();
        if (reply.value().path().isEmpty()) {
            continue;
        }
        QDBusInterface unit("org.freedesktop.systemd1", reply.value().path(), "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QDBusVariant> replyId = unit.call("Get", "org.freedesktop.systemd1.Unit", "Id");
        if (!replyId.isValid()) {
            continue;
        }
        QString id = replyId.value().variant().toString();
        if (!id.endsWith("service")) {
            qWarning() << "check startdde error: is not a service.";
            return false;
        }
    }
    return true;
}

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
        qInfo() << "session exit...";
        if (checkStartddeSession()) {
            QDBusInterface systemd("org.freedesktop.systemd1", "/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager");
            qInfo() << systemd.call("StopUnit", "dbus.service", "replace-irreversibly");
        }
        return 0;
    }

    return 0;
}
