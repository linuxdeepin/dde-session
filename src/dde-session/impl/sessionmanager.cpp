#include "sessionmanager.h"
#include "utils/dconf.h"

#include <QCoreApplication>
#include <QDBusReply>
#include <QDBusObjectPath>

#include <unistd.h>


SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , m_locked(false)
    , m_stage(0)
    , m_currentSessionPath("/")
    , m_powerInter(nullptr)
    , m_login1ManagerInter(nullptr)
    , m_login1SessionSelfInter(nullptr)
    , m_login1SessionInter(nullptr)
    , m_DbusInter(nullptr)
{
    init();
}

void SessionManager::init()
{
    m_powerInter = new com::deepin::daemon::PowerManager(
        "com.deepin.daemon.PowerManager",
        "/com/deepin/daemon/PowerManager",
        QDBusConnection::systemBus(),
        this);

    m_login1ManagerInter = new org::freedesktop::login1::Manager(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        QDBusConnection::systemBus(),
        this);
    QDBusPendingReply<QDBusObjectPath> pathResult = m_login1ManagerInter->GetSessionByPID(0); // TODO
    pathResult.waitForFinished();
    if (!pathResult.isError()) {
        m_login1SessionSelfInter = new org::freedesktop::login1::Session("org.freedesktop.login1",
            pathResult.value().path(),
            QDBusConnection::systemBus(),
            this);
    }

    if (m_login1SessionSelfInter != nullptr) {
        connect(m_login1SessionSelfInter, &org::freedesktop::login1::Session::ActiveChanged, [=](bool value) {
            qDebug() << "session status changed:" << value;
            if (value) {
                // 变活跃
            } else {
                // 变不活跃
                bool isPreparingForSleep = m_login1ManagerInter->preparingForSleep();
                if (!isPreparingForSleep) {
                    qDebug() << "call objLoginSessionSelf.Lock";
                    m_login1SessionSelfInter->Lock();
                }
            }
        });
    }
    if (Dconf::GetValue("com.deepin.dde.startdde", "", "swap-sched-enabled", QVariant(false)).toBool()) {
        initSwapSched(); // TODO
    }

    m_currentUid = QString::number(getuid());

    // initInhibitManager // TODO

    m_DbusInter = new org::freedesktop::DBus(
        "org.freedesktop.DBus",
        "/org/freedesktop/DBus",
        QDBusConnection::sessionBus(),
        this);
    if (m_DbusInter != nullptr) {
        connect(m_DbusInter, &org::freedesktop::DBus::NameOwnerChanged,
        [=](const QString &name, const QString &oldOwner, const QString &newOwner) {
            if (newOwner == "" && oldOwner != "" && name == oldOwner && name.startsWith(":")) {
                // uniq name lost
                // TODO inhibitManager
            }
        });
    }
}

void SessionManager::initSwapSched()
{
    qDebug() << "init swap sched";
    // TODO
}

bool SessionManager::locked()
{
    return m_locked;
}

void SessionManager::setLocked(bool value)
{
    m_locked = value;
}

QString SessionManager::currentUid()
{
    return m_currentUid;
}

void SessionManager::setCurrentUid(QString value)
{
    m_currentUid = value;
}

int SessionManager::stage()
{
    return m_stage;
}

void SessionManager::setStage(int value)
{
    m_stage = value;
}

QDBusObjectPath SessionManager::currentSessionPath()
{
    return m_currentSessionPath;
}

void SessionManager::setCurrentSessionPath(QDBusObjectPath value)
{
    m_currentSessionPath = value;
}

bool SessionManager::AllowSessionDaemonRun()
{
    // TODO: run com.deepin.daemon.Daemon.StartPart2
    return true;
}

bool SessionManager::CanHibernate()
{
    if (QString(getenv("POWER_CAN_SLEEP")) == "0") {
        return false;
    }
    bool isCanHibernate = m_powerInter->CanHibernate();
    if (!isCanHibernate) {
         return false;
    }
    QString resultLogin = m_login1ManagerInter->CanHibernate();
    if (resultLogin != "yes") {
        return false;
    }
    return true;
}

bool SessionManager::CanLogout()
{
    return true;
}

bool SessionManager::CanReboot()
{
    QString resultLogin = m_login1ManagerInter->CanReboot();
    if (resultLogin != "yes") {
        return false;
    }
    return true;
}

bool SessionManager::CanShutdown()
{
    QString resultLogin = m_login1ManagerInter->CanPowerOff();
    if (resultLogin != "yes") {
        return false;
    }
    return true;
}

bool SessionManager::CanSuspend()
{
    if (QString(getenv("POWER_CAN_SLEEP")) == "0") {
        return false;
    }
    bool isCanSuspend = m_powerInter->CanSuspend();
    if (!isCanSuspend) {
         return false;
    }
    QString resultLogin = m_login1ManagerInter->CanSuspend();
    if (resultLogin != "yes") {
        return false;
    }
    return true;
}

void SessionManager::ForceLogout()
{
    qInfo() << "force logout by session manager";
    // TODO:
}

void SessionManager::ForceReboot()
{

}

void SessionManager::ForceShutdown()
{

}

void SessionManager::Logout()
{

}

void SessionManager::PowerOffChoose()
{

}

void SessionManager::Reboot()
{

}

bool SessionManager::Register(const QString &id)
{
    Q_UNUSED(id)
    return false;
}

void SessionManager::RequestHibernate()
{
    m_login1ManagerInter->Hibernate(false);
    // TODO
}

void SessionManager::RequestLock()
{

}

void SessionManager::RequestLogout()
{

}

void SessionManager::RequestReboot()
{

}

void SessionManager::RequestShutdown()
{

}

void SessionManager::RequestSuspend()
{

}

void SessionManager::SetLocked(bool value)
{
    Q_UNUSED(value)
}

void SessionManager::Shutdown()
{

}

void SessionManager::ToggleDebug()
{

}

void SessionManager::prepareShutdown(bool isForce)
{
    Q_UNUSED(isForce)
}

void SessionManager::clearCurrentTty()
{

}

