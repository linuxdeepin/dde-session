// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lockprocessmanager.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <systemd/sd-daemon.h>

#include <DLog>

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-quick-login");

    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin Lock Quick Login");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption timeoutOption("t", "Timeout in seconds", "timeout");
    parser.addOption(timeoutOption);
    parser.process(app);

    DLogManager::registerJournalAppender();

    // 检测是否有传timeout参数，如果有则获取，没有则是默认值300s
    int timeout = 60; // Default timeout: 1 minutes
    if (parser.isSet(timeoutOption)) {
        timeout = parser.value(timeoutOption).toInt();
    }

    // Initialize components
    LockProcessManager processManager(timeout);
    QObject::connect(&processManager, &LockProcessManager::sessionLocked, &app, [&app](bool locked) {
        qInfo() << "Session locked status changed:" << locked;
        if (locked) {
            // 如果session被锁定，发送READY通知
            sd_notify(0, "READY=1");
        } else {
            // 如果session被解锁，退出程序，dde-lock交给systemd管理
            qInfo() << "Session unlocked, exiting application.";
            app.quit();
        }
    });
    processManager.start();

    return app.exec();
}
