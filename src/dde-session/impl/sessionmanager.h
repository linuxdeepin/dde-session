#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <QDBusObjectPath>
#include <QDBusInterface>

#include "com_deepin_PowerManager.h"
#include "org_freedesktop_login1_Manager.h"
#include "org_freedesktop_login1_Session.h"
#include "org_freedesktop_DBus.h"


class SessionManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool Locked READ locked WRITE setLocked NOTIFY lockedChanged)
    Q_PROPERTY(QString CurrentUid READ currentUid WRITE setCurrentUid NOTIFY currentUidChanged)
    Q_PROPERTY(int Stage READ stage WRITE setStage NOTIFY stageChanged)
    Q_PROPERTY(QDBusObjectPath CurrentSessionPath READ currentSessionPath WRITE setCurrentSessionPath NOTIFY currentSessionPathChanged)

public:
    explicit SessionManager(QObject *parent = nullptr);

    bool locked();
    void setLocked(bool value);
    QString currentUid();
    void setCurrentUid(QString value);
    int stage();
    void setStage(int value);
    QDBusObjectPath currentSessionPath();
    void setCurrentSessionPath(QDBusObjectPath value);

public Q_SLOTS:
    bool AllowSessionDaemonRun();
    bool CanHibernate();
    bool CanLogout();
    bool CanReboot();
    bool CanShutdown();
    bool CanSuspend();
    void ForceLogout();
    void ForceReboot();
    void ForceShutdown();
    void Logout();
    void PowerOffChoose();
    void Reboot();
    bool Register(const QString &id);
    void RequestHibernate();
    void RequestLock();
    void RequestLogout();
    void RequestReboot();
    void RequestShutdown();
    void RequestSuspend();
    void SetLocked(bool value);
    void Shutdown();
    void ToggleDebug();

private:
    // not dbus
    void init();
    void initSwapSched();
    void prepareShutdown(bool isForce);
    void clearCurrentTty();

signals:
    void Unlock();

    void lockedChanged(bool);
    void currentUidChanged(QString);
    void stageChanged(int);
    void currentSessionPathChanged(QDBusObjectPath);

private:
    bool m_locked;
    QString m_currentUid;
    int m_stage;
    QDBusObjectPath m_currentSessionPath;

    // dbus client
    com::deepin::daemon::PowerManager *m_powerInter;
    org::freedesktop::login1::Manager *m_login1ManagerInter;
    org::freedesktop::login1::Session *m_login1SessionSelfInter;
    org::freedesktop::login1::Session *m_login1SessionInter;
    org::freedesktop::DBus *m_DbusInter;
};

#endif // SESSIONMANAGER_H
