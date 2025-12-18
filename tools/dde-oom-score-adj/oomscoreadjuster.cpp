// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "oomscoreadjuster.h"
#include <DConfig>
#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <unistd.h>
#include <sys/types.h>

DCORE_USE_NAMESPACE

OomScoreAdjuster::OomScoreAdjuster(QObject *parent)
    : QObject(parent)
    , m_systemdBus(QDBusConnection::sessionBus())
    , m_exitTimer(new QTimer(this))
    , m_targetScore(-999)
{
    loadServiceList();
    
    // 设置3分钟后自动退出
    m_exitTimer->setSingleShot(true);
    m_exitTimer->setInterval(3 * 60 * 1000); // 3分钟 = 180000毫秒
    connect(m_exitTimer, &QTimer::timeout, this, &OomScoreAdjuster::onExitTimeout);
}

OomScoreAdjuster::~OomScoreAdjuster()
{
}

void OomScoreAdjuster::loadServiceList()
{
    // 从 DConfig 读取服务列表
    // 注意：传入 this 作为 parent，Qt 会自动管理内存，不需要手动 delete
    DConfig *config = DConfig::create("org.deepin.dde.session", "org.deepin.dde.session.oom-score-adj", QString(), this);
    
    if (config && config->isValid()) {
        // 读取服务列表
        m_watchedServices = config->value("monitorServices").toStringList();
    } else {
        qWarning() << "Failed to load configuration, using default services";
        m_watchedServices << "dde-session-manager.service" << "dde-session@x11.service";
    }
    if (m_watchedServices.isEmpty()) {
        qWarning() << "No services to monitor, exiting immediately";
        emit handleExit();
        return;
    }
}

void OomScoreAdjuster::start()
{
    // 检查 D-Bus 连接
    if (!m_systemdBus.isConnected()) {
        qWarning() << "D-Bus connection failed, exiting immediately";
        emit handleExit();
        return;
    }
    
    qInfo() << "Starting OOM score adjuster for services:" << m_watchedServices;
    
    // 订阅 systemd 信号
    subscribeToSystemd();
    
    // 检查已经运行的服务
    checkExistingServices();
    
    // 检查是否所有服务都已调整完成
    if (m_adjustedServices.size() == m_watchedServices.size()) {
        qInfo() << "All services have been adjusted successfully, exiting immediately";
        qInfo() << "Adjusted services:" << m_adjustedServices;
        emit handleExit();
        return;
    }
    
    // 还有服务未调整，启动 3 分钟超时定时器
    m_exitTimer->start();
    qInfo() << "Waiting for remaining services. Adjusted:" << m_adjustedServices.size() 
            << "/ Total:" << m_watchedServices.size();
    qInfo() << "Timeout timer started, will exit in 3 minutes if not all services are adjusted";
}

void OomScoreAdjuster::subscribeToSystemd()
{
    // 连接到 systemd 的 UnitNew 信号
    m_systemdBus.connect(
        "org.freedesktop.systemd1",
        "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager",
        "UnitNew",
        this,
        SLOT(onUnitNew(QString, QDBusObjectPath))
    );
}

void OomScoreAdjuster::checkExistingServices()
{
    for (const QString &serviceName : m_watchedServices) {
        checkAndAdjustService(serviceName);
    }
}

void OomScoreAdjuster::onUnitNew(const QString &unitName, const QDBusObjectPath &unitPath)
{
    if (m_watchedServices.contains(unitName)) {
        m_servicePathMap[unitName] = unitPath.path();
        
        // 检查服务当前状态
        QDBusInterface unit(
            "org.freedesktop.systemd1",
            unitPath.path(),
            "org.freedesktop.systemd1.Unit",
            m_systemdBus
        );
        
        if (unit.isValid()) {
            QString activeState = unit.property("ActiveState").toString();
            
            if (activeState == "active") {
                // 服务已经是 active，直接调整，不需要监听信号
                checkAndAdjustService(unitName);
            } else {
                // 服务还未 active，监听状态变化
                m_systemdBus.connect(
                    "org.freedesktop.systemd1",
                    unitPath.path(),
                    "org.freedesktop.DBus.Properties",
                    "PropertiesChanged",
                    this,
                    SLOT(onPropertiesChanged(QString, QVariantMap, QStringList))
                );
                qInfo() << "Service" << unitName << "is not active yet, waiting for activation";
            }
        }
    }
}

void OomScoreAdjuster::onPropertiesChanged(const QString &interface,
                                           const QVariantMap &changedProperties,
                                           const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties)
    
    if (interface != "org.freedesktop.systemd1.Service" && interface != "org.freedesktop.systemd1.Unit") {
        return;
    }
    
    if (!changedProperties.contains("ActiveState")) {
        return;
    }
    
    QString activeState = changedProperties.value("ActiveState").toString();
    if (activeState != "active") {
        return;
    }
    
    for (const QString &serviceName : m_watchedServices) {
        checkAndAdjustService(serviceName);
    }
}

void OomScoreAdjuster::checkAndAdjustService(const QString &serviceName)
{
    QList<quint32> pids = getServiceProcesses(serviceName);
    if (pids.isEmpty()) {
        qDebug() << "Service" << serviceName << "has no processes yet, waiting for it to start";
        return;
    }
    
    bool allAdjusted = true;
    for (quint32 pid : pids) {
        // 读取当前的 oom_score_adj
        QString procPath = QString("/proc/%1/oom_score_adj").arg(pid);
        QFile readFile(procPath);
        if (readFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString currentScore = readFile.readAll().trimmed();
            readFile.close();
            
            // 如果已经是目标值，跳过
            if (currentScore.toInt() == m_targetScore) {
                continue;
            }
        }
        
        if (adjustOomScore(pid, m_targetScore)) {
            qInfo() << "Successfully adjusted OOM score for" << serviceName << "PID:" << pid << "to" << m_targetScore;
        } else {
            qWarning() << "Failed to adjust OOM score for" << serviceName << "PID:" << pid;
            allAdjusted = false;
        }
    }
    
    // 标记该服务已完成调整
    if (allAdjusted && !pids.isEmpty()) {
        m_adjustedServices.insert(serviceName);
        qInfo() << "Service" << serviceName << "adjustment completed";
        
        // 检查是否所有服务都已调整完成
        checkIfAllServicesAdjusted();
    }
}

QList<quint32> OomScoreAdjuster::getServiceProcesses(const QString &serviceName)
{
    QList<quint32> pids;
    
    // 获取 unit 的对象路径
    QString unitPath = getUnitObjectPath(serviceName);
    if (unitPath.isEmpty()) {
        return pids;
    }
    
    // 调用 GetProcesses 方法
    QDBusInterface service(
        "org.freedesktop.systemd1",
        unitPath,
        "org.freedesktop.systemd1.Service",
        m_systemdBus
    );
    
    if (!service.isValid()) {
        return pids;
    }
    
    QDBusMessage reply = service.call("GetProcesses");
    if (reply.type() == QDBusMessage::ErrorMessage) {
        return pids;
    }
    
    if (reply.arguments().isEmpty()) {
        return pids;
    }
    
    // 解析返回值: a(sus) - array of (string, uint32, string)
    const QDBusArgument arg = reply.arguments().at(0).value<QDBusArgument>();
    
    arg.beginArray();
    while (!arg.atEnd()) {
        QString cgroupPath;
        quint32 pid = 0;
        QString command;
        
        arg.beginStructure();
        arg >> cgroupPath >> pid >> command;
        arg.endStructure();
        
        if (pid > 0) {
            pids.append(pid);
        }
    }
    arg.endArray();
        
    return pids;
}

QString OomScoreAdjuster::getUnitObjectPath(const QString &serviceName)
{
    // 如果已经缓存了路径，直接返回
    if (m_servicePathMap.contains(serviceName)) {
        return m_servicePathMap[serviceName];
    }
    
    // 通过 systemd Manager 接口获取 unit 路径
    QDBusInterface manager(
        "org.freedesktop.systemd1",
        "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager",
        m_systemdBus
    );
    
    if (!manager.isValid()) {
        return QString();
    }
    
    QDBusReply<QDBusObjectPath> reply = manager.call("GetUnit", serviceName);
    if (!reply.isValid()) {
        return QString();
    }
    
    QString path = reply.value().path();
    m_servicePathMap[serviceName] = path;
    
    return path;
}

bool OomScoreAdjuster::adjustOomScore(quint32 pid, int score)
{
    if (pid == 0) {
        return false;
    }
    
    QString procPath = QString("/proc/%1/oom_score_adj").arg(pid);
    
    QFile file(procPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Unbuffered)) {
        return false;
    }
    
    QTextStream out(&file);
    out << score;
    out.flush();
    file.close();
    
    if (file.error() != QFile::NoError) {
        return false;
    }
    
    // 验证写入
    QFile verifyFile(procPath);
    if (verifyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString actualValue = verifyFile.readAll().trimmed();
        verifyFile.close();
        return actualValue.toInt() == score;
    }
    
    return false;
}

void OomScoreAdjuster::checkIfAllServicesAdjusted()
{
    if (m_adjustedServices.size() == m_watchedServices.size()) {
        qInfo() << "All services have been adjusted successfully, exiting immediately";
        qInfo() << "Adjusted services:" << m_adjustedServices;
        
        // 停止定时器
        m_exitTimer->stop();
        
        // 立即退出
        emit handleExit();
    } else {
        qInfo() << "Waiting for remaining services. Adjusted:" << m_adjustedServices.size() 
                << "/ Total:" << m_watchedServices.size();
    }
}

void OomScoreAdjuster::onExitTimeout()
{
    qWarning() << "Exit timeout reached (3 minutes), exiting now";
    qWarning() << "Adjusted services:" << m_adjustedServices.size() << "/" << m_watchedServices.size();
    qWarning() << "Successfully adjusted:" << m_adjustedServices;
    
    QSet<QString> pending;
    for (const QString &service : m_watchedServices) {
        if (!m_adjustedServices.contains(service)) {
            pending.insert(service);
        }
    }
    if (!pending.isEmpty()) {
        qWarning() << "Pending services:" << pending;
    }
    
    emit handleExit();
}
