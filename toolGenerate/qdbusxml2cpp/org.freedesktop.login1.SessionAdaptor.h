/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp ./dde-session/dbus/interface/org.freedesktop.login1.Session.xml -a ./dde-session/toolGenerate/qdbusxml2cpp/org.freedesktop.login1.SessionAdaptor -i ./dde-session/toolGenerate/qdbusxml2cpp/org.freedesktop.login1.Session.h
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef ORG_FREEDESKTOP_LOGIN1_SESSIONADAPTOR_H
#define ORG_FREEDESKTOP_LOGIN1_SESSIONADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "./dde-session/toolGenerate/qdbusxml2cpp/org.freedesktop.login1.Session.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.freedesktop.login1.Session
 */
class SessionAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.login1.Session")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.freedesktop.login1.Session\">\n"
"    <property access=\"read\" type=\"s\" name=\"Id\">\n"
"      <annotation value=\"const\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"s\" name=\"Name\">\n"
"      <annotation value=\"const\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"t\" name=\"Timestamp\">\n"
"      <annotation value=\"const\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"t\" name=\"TimestampMonotonic\">\n"
"      <annotation value=\"const\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"u\" name=\"VTNr\">\n"
"      <annotation value=\"const\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"s\" name=\"TTY\">\n"
"      <annotation value=\"const\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"s\" name=\"Display\">\n"
"      <annotation value=\"const\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"b\" name=\"Remote\">\n"
"      <annotation value=\"const\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"s\" name=\"RemoteHost\">\n"
"      <annotation value=\"const\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"s\" name=\"RemoteUser\">\n"
"      <annotation value=\"const\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"s\" name=\"Service\">\n"
"      <annotation value=\"const\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"s\" name=\"Desktop\">\n"
"      <annotation value=\"const\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"s\" name=\"Scope\">\n"
"      <annotation value=\"const\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"u\" name=\"Leader\">\n"
"      <annotation value=\"const\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"u\" name=\"Audit\">\n"
"      <annotation value=\"const\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"s\" name=\"Type\">\n"
"      <annotation value=\"const\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"b\" name=\"Active\"/>\n"
"    <property access=\"read\" type=\"s\" name=\"State\">\n"
"      <annotation value=\"false\" name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"b\" name=\"IdleHint\"/>\n"
"    <property access=\"read\" type=\"t\" name=\"IdleSinceHint\"/>\n"
"    <property access=\"read\" type=\"t\" name=\"IdleSinceHintMonotonic\"/>\n"
"    <property access=\"read\" type=\"b\" name=\"LockedHint\"/>\n"
"    <method name=\"Terminate\"/>\n"
"    <method name=\"Activate\"/>\n"
"    <method name=\"Lock\"/>\n"
"    <method name=\"Unlock\"/>\n"
"    <method name=\"SetIdleHint\">\n"
"      <arg direction=\"in\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"SetLockedHint\">\n"
"      <arg direction=\"in\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"Kill\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"i\"/>\n"
"    </method>\n"
"    <method name=\"TakeControl\">\n"
"      <arg direction=\"in\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"ReleaseControl\"/>\n"
"    <method name=\"TakeDevice\">\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"      <arg direction=\"out\" type=\"h\"/>\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"ReleaseDevice\">\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"    </method>\n"
"    <method name=\"PauseDeviceComplete\">\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"    </method>\n"
"    <signal name=\"PauseDevice\">\n"
"      <arg type=\"u\"/>\n"
"      <arg type=\"u\"/>\n"
"      <arg type=\"s\"/>\n"
"    </signal>\n"
"    <signal name=\"ResumeDevice\">\n"
"      <arg type=\"u\"/>\n"
"      <arg type=\"u\"/>\n"
"      <arg type=\"h\"/>\n"
"    </signal>\n"
"  </interface>\n"
        "")
public:
    SessionAdaptor(QObject *parent);
    virtual ~SessionAdaptor();

public: // PROPERTIES
    Q_PROPERTY(bool Active READ active)
    bool active() const;

    Q_PROPERTY(uint Audit READ audit)
    uint audit() const;

    Q_PROPERTY(QString Desktop READ desktop)
    QString desktop() const;

    Q_PROPERTY(QString Display READ display)
    QString display() const;

    Q_PROPERTY(QString Id READ id)
    QString id() const;

    Q_PROPERTY(bool IdleHint READ idleHint)
    bool idleHint() const;

    Q_PROPERTY(qulonglong IdleSinceHint READ idleSinceHint)
    qulonglong idleSinceHint() const;

    Q_PROPERTY(qulonglong IdleSinceHintMonotonic READ idleSinceHintMonotonic)
    qulonglong idleSinceHintMonotonic() const;

    Q_PROPERTY(uint Leader READ leader)
    uint leader() const;

    Q_PROPERTY(bool LockedHint READ lockedHint)
    bool lockedHint() const;

    Q_PROPERTY(QString Name READ name)
    QString name() const;

    Q_PROPERTY(bool Remote READ remote)
    bool remote() const;

    Q_PROPERTY(QString RemoteHost READ remoteHost)
    QString remoteHost() const;

    Q_PROPERTY(QString RemoteUser READ remoteUser)
    QString remoteUser() const;

    Q_PROPERTY(QString Scope READ scope)
    QString scope() const;

    Q_PROPERTY(QString Service READ service)
    QString service() const;

    Q_PROPERTY(QString State READ state)
    QString state() const;

    Q_PROPERTY(QString TTY READ tTY)
    QString tTY() const;

    Q_PROPERTY(qulonglong Timestamp READ timestamp)
    qulonglong timestamp() const;

    Q_PROPERTY(qulonglong TimestampMonotonic READ timestampMonotonic)
    qulonglong timestampMonotonic() const;

    Q_PROPERTY(QString Type READ type)
    QString type() const;

    Q_PROPERTY(uint VTNr READ vTNr)
    uint vTNr() const;

public Q_SLOTS: // METHODS
    void Activate();
    void Kill(const QString &in0, int in1);
    void Lock();
    void PauseDeviceComplete(uint in0, uint in1);
    void ReleaseControl();
    void ReleaseDevice(uint in0, uint in1);
    void SetIdleHint(bool in0);
    void SetLockedHint(bool in0);
    void TakeControl(bool in0);
    QDBusUnixFileDescriptor TakeDevice(uint in0, uint in1, bool &out1);
    void Terminate();
    void Unlock();
Q_SIGNALS: // SIGNALS
    void PauseDevice(uint in0, uint in1, const QString &in2);
    void ResumeDevice(uint in0, uint in1, const QDBusUnixFileDescriptor &in2);
};

#endif