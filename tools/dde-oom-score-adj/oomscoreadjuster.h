// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OOMSCOREADJUSTER_H
#define OOMSCOREADJUSTER_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QTimer>

/**
 * @brief The OomScoreAdjuster class
 * @brief 监听 systemd 服务启动并调整其 OOM score
 */
class OomScoreAdjuster : public QObject
{
    Q_OBJECT

public:
    explicit OomScoreAdjuster(QObject *parent = nullptr);
    ~OomScoreAdjuster();

signals:
    void handleExit();

public slots:
    void start();

private slots:
    void onExitTimeout();
    void onUnitNew(const QString &unitName, const QDBusObjectPath &unitPath);
    void onPropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

private:
    void loadServiceList();
    void subscribeToSystemd();
    void checkExistingServices();
    void checkAndAdjustService(const QString &serviceName);
    bool adjustOomScore(quint32 pid, int score);
    QList<quint32> getServiceProcesses(const QString &serviceName);
    QString getUnitObjectPath(const QString &serviceName);
    void checkIfAllServicesAdjusted();

private:
    QDBusConnection m_systemdBus;
    QStringList m_watchedServices;
    QMap<QString, QString> m_servicePathMap; // serviceName -> objectPath
    QSet<QString> m_adjustedServices; // 已完成调整的服务
    int m_targetScore;
    QTimer *m_exitTimer;
};

#endif // OOMSCOREADJUSTER_H
