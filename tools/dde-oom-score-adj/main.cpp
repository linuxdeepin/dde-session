// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "oomscoreadjuster.h"
#include <QCoreApplication>
#include <QDebug>
#include <cap-ng.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 初始化 libcap-ng
    capng_get_caps_process();
    
    // 检查是否有 CAP_SYS_RESOURCE capability（仅用于调试信息）
    if (!capng_have_capability(CAPNG_EFFECTIVE, CAP_SYS_RESOURCE)) {
        qWarning() << "Warning: Missing CAP_SYS_RESOURCE capability!";
        qWarning() << "The program may not be able to modify oom_score_adj.";
        qWarning() << "When running as systemd service, capabilities are granted automatically.";
    }
    
    OomScoreAdjuster adjuster;
    
    // 连接退出信号
    QObject::connect(&adjuster, &OomScoreAdjuster::handleExit, &app, &QCoreApplication::quit);
    QMetaObject::invokeMethod(&adjuster, "start", Qt::QueuedConnection);
    
    return app.exec();
}
