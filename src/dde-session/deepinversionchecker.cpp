#include "deepinversionchecker.h"

#include <QSettings>
#include <QDebug>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QCoreApplication>

bool isDeepinVersionChanged()
{
    // 查看系统当前版本
    QSettings osSettings("/etc/deepin-version", QSettings::IniFormat);
    osSettings.beginGroup("Release");
    const QString &osVersion = osSettings.value("Version").toString();

    const QString &welcomePath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()
            + QDir::separator()
            + qApp->organizationName()
            + QDir::separator();
    const QString &welcomeConf = "dde-welcome.conf";

    // 将版本信息写入欢迎应用的配置文件中
    auto saveConfig = [ = ] {
        if (!QDir(welcomePath).exists()) {
            bool ok = QDir().mkpath(welcomePath);
            if (!ok) {
                qWarning() << "create welcome config path failed.";
            }
        }

        QSettings osSettings(welcomePath + welcomeConf, QSettings::IniFormat);
        osSettings.setValue("Version", osVersion);
        osSettings.sync();
    };

    if (!QFile(welcomePath + welcomeConf).exists()) {
        saveConfig();
        return true;
    }

    // 比较系统版本和配置文件中记录的是否一致
    QSettings welcomeSettings(welcomePath + welcomeConf, QSettings::IniFormat);
    const QString &welcomeVersion = welcomeSettings.value("Version").toString();
    bool changed =  (osVersion != welcomeVersion);
    qDebug() << "os version: " << osVersion
             << ", welcome version: " << welcomeVersion
             << ", version changed: " << changed;
    if (changed)
        saveConfig();

    return changed;
}

void maybeOpenWelcome()
{
    if (!isDeepinVersionChanged()) {
        return;
    }

    // TODO 如果当前正在回滚系统，不应该显示欢迎界面，以前是通过ABRecovery来完成的，V23废弃了这个项目，这里的逻辑待定

    qDebug() << "open the welcome";
    QProcess::startDetached("/usr/lib/deepin-daemon/dde-welcome");
}

void DeepinVersionChecker::init()
{
    qInfo() << "deepin version checker init";

    maybeOpenWelcome();

    qInfo() << "deepin version checker init finished";
}
