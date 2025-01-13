/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp ./dde-session/dbus/adaptor/org.deepin.dde.SessionManager1.xml -a ./dde-session/toolGenerate/qdbusxml2cpp/org.deepin.dde.SessionManager1Adaptor -i ./dde-session/toolGenerate/qdbusxml2cpp/org.deepin.dde.SessionManager1.h
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef ORG_DEEPIN_DDE_SESSIONMANAGER1ADAPTOR_H
#define ORG_DEEPIN_DDE_SESSIONMANAGER1ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "./dde-session/toolGenerate/qdbusxml2cpp/org.deepin.dde.SessionManager1.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.deepin.dde.SessionManager1
 */
class SessionManager1Adaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.SessionManager1")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.dde.SessionManager1\">\n"
"    <method name=\"AllowSessionDaemonRun\">\n"
"      <arg direction=\"out\" type=\"b\" name=\"result\"/>\n"
"    </method>\n"
"    <method name=\"CanHibernate\">\n"
"      <arg direction=\"out\" type=\"b\" name=\"result\"/>\n"
"    </method>\n"
"    <method name=\"CanLogout\">\n"
"      <arg direction=\"out\" type=\"b\" name=\"result\"/>\n"
"    </method>\n"
"    <method name=\"CanReboot\">\n"
"      <arg direction=\"out\" type=\"b\" name=\"result\"/>\n"
"    </method>\n"
"    <method name=\"CanShutdown\">\n"
"      <arg direction=\"out\" type=\"b\" name=\"result\"/>\n"
"    </method>\n"
"    <method name=\"CanSuspend\">\n"
"      <arg direction=\"out\" type=\"b\" name=\"result\"/>\n"
"    </method>\n"
"    <method name=\"ForceLogout\"/>\n"
"    <method name=\"ForceReboot\"/>\n"
"    <method name=\"ForceShutdown\"/>\n"
"    <method name=\"GetInhibitors\">\n"
"      <arg direction=\"out\" type=\"ao\" name=\"Inhibitors\"/>\n"
"    </method>\n"
"    <method name=\"Inhibit\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"appId\"/>\n"
"      <arg direction=\"in\" type=\"u\" name=\"toplevelXid\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"reason\"/>\n"
"      <arg direction=\"in\" type=\"u\" name=\"flags\"/>\n"
"      <arg direction=\"out\" type=\"u\" name=\"inhibitCookie\"/>\n"
"    </method>\n"
"    <method name=\"IsInhibited\">\n"
"      <arg direction=\"in\" type=\"u\" name=\"flags\"/>\n"
"      <arg direction=\"out\" type=\"b\" name=\"ret\"/>\n"
"    </method>\n"
"    <method name=\"Uninhibit\">\n"
"      <arg direction=\"in\" type=\"u\" name=\"inhibitCookie\"/>\n"
"    </method>\n"
"    <method name=\"Logout\"/>\n"
"    <method name=\"PowerOffChoose\"/>\n"
"    <method name=\"Reboot\"/>\n"
"    <method name=\"Register\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"id\"/>\n"
"      <arg direction=\"out\" type=\"b\" name=\"result\"/>\n"
"    </method>\n"
"    <method name=\"RequestHibernate\"/>\n"
"    <method name=\"RequestLock\"/>\n"
"    <method name=\"RequestLogout\"/>\n"
"    <method name=\"RequestReboot\"/>\n"
"    <method name=\"RequestShutdown\"/>\n"
"    <method name=\"RequestSuspend\"/>\n"
"    <method name=\"SetLocked\">\n"
"      <arg direction=\"in\" type=\"b\" name=\"value\"/>\n"
"    </method>\n"
"    <method name=\"Shutdown\"/>\n"
"    <method name=\"ToggleDebug\"/>\n"
"    <signal name=\"Unlock\"/>\n"
"    <signal name=\"InhibitorAdded\">\n"
"      <arg type=\"o\" name=\"path\"/>\n"
"    </signal>\n"
"    <signal name=\"InhibitorRemoved\">\n"
"      <arg type=\"o\" name=\"path\"/>\n"
"    </signal>\n"
"    <property access=\"read\" type=\"b\" name=\"Locked\"/>\n"
"    <property access=\"read\" type=\"s\" name=\"CurrentUid\"/>\n"
"    <property access=\"read\" type=\"i\" name=\"Stage\"/>\n"
"    <property access=\"read\" type=\"o\" name=\"CurrentSessionPath\"/>\n"
"  </interface>\n"
        "")
public:
    SessionManager1Adaptor(QObject *parent);
    virtual ~SessionManager1Adaptor();

public: // PROPERTIES
    Q_PROPERTY(QDBusObjectPath CurrentSessionPath READ currentSessionPath)
    QDBusObjectPath currentSessionPath() const;

    Q_PROPERTY(QString CurrentUid READ currentUid)
    QString currentUid() const;

    Q_PROPERTY(bool Locked READ locked)
    bool locked() const;

    Q_PROPERTY(int Stage READ stage)
    int stage() const;

public Q_SLOTS: // METHODS
    bool AllowSessionDaemonRun();
    bool CanHibernate();
    bool CanLogout();
    bool CanReboot();
    bool CanShutdown();
    bool CanSuspend();
    void ForceLogout();
    void ForceReboot();
    void ForceShutdown();
    QList<QDBusObjectPath> GetInhibitors();
    uint Inhibit(const QString &appId, uint toplevelXid, const QString &reason, uint flags);
    bool IsInhibited(uint flags);
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
    void Uninhibit(uint inhibitCookie);
Q_SIGNALS: // SIGNALS
    void InhibitorAdded(const QDBusObjectPath &path);
    void InhibitorRemoved(const QDBusObjectPath &path);
    void Unlock();
};

#endif
