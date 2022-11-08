#include "loginremindermanager.h"
#include "utils.h"
#include "loginreminderinfo.h"

#include <unistd.h>

#include <QDateTime>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QDBusPendingReply>
#include <QNetworkInterface>
#include <QProcess>
#include <QCoreApplication>

LoginReminderManager::LoginReminderManager(QObject *parent)
    : QObject(parent)
    , m_notifyId(0)
{
    registerLoginReminderInfoMetaType();
}

void LoginReminderManager::init()
{
    qDebug() << "login reminder init";

    if (!Utils::SettingValue("com.deepin.dde.startdde", QByteArray(), "login-reminder", false).toBool())
        qApp->quit();

    // 登录后展示横幅通知信息
#ifdef QT_DEBUG // V20 验证
    QDBusInterface userInter("com.deepin.daemon.Accounts", QString("/com/deepin/daemon/Accounts/User%1").arg(getuid()), "com.deepin.daemon.Accounts.User", QDBusConnection::systemBus());
#else
    QDBusInterface userInter("org.deepin.daemon.Accounts1", QString("/org/deepin/daemon/Accounts1/User%1").arg(getuid()), "org.deepin.daemon.Accounts1.User", QDBusConnection::systemBus());
#endif
    QDBusPendingReply<LoginReminderInfo> reply = userInter.call("GetReminderInfo");
    if (reply.isError()) {
        qWarning() << "failed to retrieve login reminder info, error: " << reply.error().message();
        exit(-1);
    }

    // 获取当前设备的ip
    auto getFirstIpAddress = [] {
        QList<QHostAddress> addressesList = QNetworkInterface::allAddresses();

        // 优先查找ipv4的地址
        auto ipV4Iter = std::find_if(addressesList.begin(), addressesList.end(), [ = ] (QHostAddress address) {
            return (address.protocol() == QAbstractSocket::IPv4Protocol)
                    && (address != QHostAddress::LocalHost)
                    && (address != QHostAddress::Broadcast)
                    && (address != QHostAddress::AnyIPv4)
                    && (address != QHostAddress::Any);
        });

        if (ipV4Iter != addressesList.end()) {
            return ipV4Iter->toString();
        }

        // 其次,寻找可用的ipv6地址
        auto ipV6Iter = std::find_if(addressesList.begin(), addressesList.end(), [ = ] (QHostAddress address) {
            return (address.protocol() == QAbstractSocket::IPv6Protocol)
                    && (address != QHostAddress::LocalHostIPv6)
                    && (address != QHostAddress::AnyIPv6)
                    && (address != QHostAddress::Any);
        });

        if (ipV6Iter != addressesList.end()) {
            return ipV6Iter->toString();
        }

        return QString();
    };

#define SECONDS_PER_DAY (60 * 60 * 24)
    const LoginReminderInfo &info = reply.value();

    const QString &m_title = tr("Login Reminder");
    const QString &currentLoginTime = info.CurrentLogin.Time.left(QString("yyyy-MM-dd hh:mm:ss").length());
    const QString &address = (info.CurrentLogin.Address == "0.0.0.0") ? getFirstIpAddress() : info.CurrentLogin.Address;
    QString body = QString("%1 %2 %3").arg(info.Username).arg(address).arg(currentLoginTime);
    int daysLeft = info.spent.LastChange + info.spent.Max - int(QDateTime::currentDateTime().toTime_t() / SECONDS_PER_DAY);
    if ((info.spent.Max != -1) && (info.spent.Warn != -1) && (info.spent.Warn > daysLeft)) {
        body += " " + QString(tr("Your password will expire in %1 days")).arg(daysLeft);
    }
    body += "\n" + QString(tr("%1 login failures since the last successful login")).arg(info.FailCountSinceLastLogin);

    const QString &lastLoginTime = info.LastLogin.Time.left(QString("yyyy-MM-dd hh:mm:ss").length());
    m_content += QString("<p>%1</p>").arg(info.Username);
    m_content += QString("<p>%1</p>").arg(address);
    m_content += "<p>" + QString(tr("Login time: %1")).arg(currentLoginTime) + "</p>";
    m_content += "<p>" + QString(tr("Last login: %1")).arg(lastLoginTime) + "</p>";
    m_content += "<p><b>" + QString("Your password will expire in %1 days").arg(daysLeft) + "</b></p>";
    m_content += "<br>";
    m_content += "<p>" + QString("%1 login failures since the last successful login").arg(info.FailCountSinceLastLogin) + "</p>";

    QDBusInterface *notifyInter = new QDBusInterface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QDBusPendingReply<uint> notifyIdReply = notifyInter->call("Notify", "dde-control-center", uint(0), "preferences-system", m_title, body, QStringList() << "details" << tr("Details"), QVariantMap(), int(0));
    if (notifyIdReply.isError()) {
        qWarning() << "failed to call notification, error: " << notifyIdReply.error().message();
        exit(-1);
    }

    m_notifyId = notifyIdReply.value();
    connect(notifyInter, SIGNAL(ActionInvoked(uint,QString)), this, SLOT(handleActionInvoked(uint, QString)));
    connect(notifyInter, SIGNAL(NotificationClosed(uint,uint)), this, SLOT(handleNotificationClosed(uint, uint)));

    qDebug() << "login reminder init finished";
}

void LoginReminderManager::handleActionInvoked(uint id, QString action)
{
    // 监听通知上的详情按钮被点击
    if (id != m_notifyId || action != "details")
        return;

    qDebug() << "show details and quit now";

    QProcess::startDetached("/usr/bin/dde-hints-dialog", QStringList() << m_title << m_content);

    qApp->quit();
}

void LoginReminderManager::handleNotificationClosed(uint id, uint)
{
    if (id != m_notifyId)
        return;

    qDebug() << "notification closed and quit now";

    qApp->quit();
}
