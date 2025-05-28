// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCKPROCESSMANAGER_H
#define LOCKPROCESSMANAGER_H

#include <QObject>
#include <QProcess>

class QTimer;
class LockProcessManager : public QObject
{
    Q_OBJECT

public:
    explicit LockProcessManager(int timeout, QObject *parent = nullptr);
    ~LockProcessManager();

    void start();

signals:
    void sessionLocked(bool locked);

private slots:
    void onTimeout();
    void onLockStatusChanged(bool locked);

private:
    void triggerLogout();

    int m_timeout;
    bool m_wasLocked = false;

    QTimer* m_timeoutTimer;
};

#endif // LOCKPROCESSMANAGER_H
