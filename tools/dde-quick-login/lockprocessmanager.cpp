// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lockprocessmanager.h"

#include <QDebug>
#include <QTimer>
#include <QProcess>
#include <QDBusInterface>
#include <QDBusServiceWatcher>

LockProcessManager::LockProcessManager(int timeout, QObject *parent)
    : QObject(parent)
    , m_timeout(timeout)
    , m_timeoutTimer(new QTimer(this))
{    
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout,
            this, &LockProcessManager::onTimeout);

    QDBusConnection::sessionBus().connect(
        "org.deepin.dde.SessionManager1",
        "/org/deepin/dde/SessionManager1",
        "org.deepin.dde.SessionManager1",
        "LockedChanged",
        this,
        SLOT(onLockStatusChanged(bool)));
}

LockProcessManager::~LockProcessManager()
{

}

void LockProcessManager::start()
{
    m_timeoutTimer->start(m_timeout * 1000);
    QProcess::startDetached("/usr/bin/dde-lock", QStringList() << "-lq");

    auto *watcher = new QDBusServiceWatcher("org.deepin.dde.LockFront1",
                                                            QDBusConnection::sessionBus(),
                                                            QDBusServiceWatcher::WatchForUnregistration, this);
    watcher->connect(watcher, &QDBusServiceWatcher::serviceUnregistered, [&] {
        qWarning() << "lock front service exit, need logout";
        triggerLogout();
    });
}

void LockProcessManager::onTimeout()
{
    qWarning() << "lock session timeout, need logout";
    triggerLogout();
}

void LockProcessManager::onLockStatusChanged(bool locked)
{
    qInfo() << "session lock status changed:" << locked;
    if (locked) {
        m_wasLocked = true;
        m_timeoutTimer->stop();
        Q_EMIT sessionLocked(true);
    } else {
        if (m_wasLocked) {
            m_wasLocked = false;
            Q_EMIT sessionLocked(false);
        }
    }
}

void LockProcessManager::triggerLogout()
{
    qInfo() << "trigger logout";
    QDBusInterface sessionManager("org.deepin.dde.SessionManager1",
                                "/org/deepin/dde/SessionManager1",
                                "org.deepin.dde.SessionManager1",
                                QDBusConnection::sessionBus());
    sessionManager.call("RequestLogout");
}
