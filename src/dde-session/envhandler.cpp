#include "envhandler.h"
#include "org_freedesktop_systemd1_Manager.h"
#include "org_freedesktop_DBus.h"
#include "types/envinfolist.h"

bool EnvHandler::setEnv(QString env, QString value)
{
    // systemd1
    org::freedesktop::systemd1::Manager systemd1("org.freedesktop.systemd1", "/org/freedesktop/systemd1", QDBusConnection::sessionBus());
    QDBusPendingReply<void> replySystemd1 = systemd1.SetEnvironment(QStringList(env+"="+value));
    replySystemd1.waitForFinished();
    if (replySystemd1.isError()) {
        qWarning() << "set systemd1 env failed:" << env;
        return false;
    }

    // dbus
    org::freedesktop::DBus dbus("org.freedesktop.DBus", "/org/freedesktop/DBus", QDBusConnection::sessionBus());
    EnvInfoList envs;
    envs.insert(env, value);
    QDBusPendingReply<void> replyDbus = dbus.UpdateActivationEnvironment(envs);
    replyDbus.waitForFinished();
    if (replyDbus.isError()) {
        qWarning() << "set dbus env failed:" << env;
        return false;
    }
    return true;
}

bool EnvHandler::unsetEnv(QString env)
{
    // systemd1
    org::freedesktop::systemd1::Manager systemd1("org.freedesktop.systemd1", "/org/freedesktop/systemd1", QDBusConnection::sessionBus());
    QDBusPendingReply<void> replySystemd1 = systemd1.UnsetEnvironment(QStringList(env));
    replySystemd1.waitForFinished();
    if (replySystemd1.isError()) {
        qWarning() << "unset systemd1 env failed:" << env;
        return false;
    }

    // dbus
    org::freedesktop::DBus dbus("org.freedesktop.DBus", "/org/freedesktop/DBus", QDBusConnection::sessionBus());
    EnvInfoList envs;
    envs.insert(env, "");
    QDBusPendingReply<void> replyDbus = dbus.UpdateActivationEnvironment(envs);
    replyDbus.waitForFinished();
    if (replyDbus.isError()) {
        qWarning() << "unset dbus env failed:" << env;
        return false;
    }
    return true;
}

bool EnvHandler::EnvInit(QString sessionType)
{
    bool ret = false;
    if (sessionType == "x11") {
        ret = x11EnvInit();
    } else if (sessionType == "wayland") {
        ret = waylandEnvInit();
    }
    return ret;
}

bool EnvHandler::x11EnvInit()
{
    bool ret = false;
    ret = unsetEnv("WAYLAND_DISPLAY");
    return ret;
}

bool EnvHandler::waylandEnvInit()
{
    return true;
}