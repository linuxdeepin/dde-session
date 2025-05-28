// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lockprocessmanager.h"

#include <QDebug>
#include <QTimer>
#include <QProcess>
#include <QDBusInterface>

LockProcessManager::LockProcessManager(int timeout, QObject *parent)
    : QObject(parent)
    , m_timeout(timeout)
    , m_process(new QProcess(this))
    , m_timeoutTimer(new QTimer(this))
{    
    m_timeoutTimer->setSingleShot(true);

    connect(m_process, &QProcess::errorOccurred,
            this, &LockProcessManager::onProcessError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &LockProcessManager::onProcessFinished);
    connect(m_timeoutTimer, &QTimer::timeout,
            this, &LockProcessManager::onTimeout);

    QDBusConnection::sessionBus().connect(
        "org.deepin.dde.SessionManager1",
        "/org/deepin/dde/SessionManager1",
        "org.deepin.dde.SessionManager1",
        "LockedChanged",
        this,
        SLOT(onLockStatusChanged()));
}

LockProcessManager::~LockProcessManager()
{
    cleanup();
}

void LockProcessManager::start()
{
    m_timeoutTimer->start(m_timeout * 1000);
    m_process->start("/usr/bin/dde-lock", QStringList() << "-lq");
}

void LockProcessManager::onProcessError(QProcess::ProcessError error)
{
    qWarning() << "Process error:" << error;
    cleanup();
    triggerLogout();
}

void LockProcessManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // Process finished, check exit code and status，include SIGKILL and SIGTERM
    qWarning() << "Process finished with exit code:" << exitCode << "and exit status:" << exitStatus;
    if (exitStatus == QProcess::CrashExit) {
        qWarning() << "Process crashed";
        cleanup();
        triggerLogout();
    }
}

void LockProcessManager::onTimeout()
{
    qWarning() << "Process timeout";
    cleanup();
    triggerLogout();
}

void LockProcessManager::onLockStatusChanged(bool locked)
{
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

void LockProcessManager::cleanup()
{
    if (m_process->state() == QProcess::Running) {
        m_process->terminate();
        if (!m_process->waitForFinished(1000)) {
            m_process->kill();
        }
    }

    m_timeoutTimer->stop();
}

void LockProcessManager::triggerLogout()
{
    QDBusInterface sessionManager("org.deepin.dde.Session1",
                                "/org/deepin/dde/Session1",
                                "org.deepin.dde.Session1",
                                QDBusConnection::sessionBus());
    sessionManager.call("Logout");
}
