#include "sessionmanager.h"
#include "utils/dconf.h"
#include "utils/utils.h"
#include "org_deepin_daemon_Audio1_Sink.h"
#include "com_deepin_api_SoundThemePlayer.h"

#include <QCoreApplication>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QGSettings>
#include <QStandardPaths>
#include <QJsonDocument>

#include <unistd.h>
#include <signal.h>

#define MASK_SERVICE(service) \
{\
    auto reply = m_systemd1ManagerInter->MaskUnitFiles(QStringList() << service, true, true);\
    if (reply.isError()) {\
    qWarning() << "failed to mask service: " << service << ", error: " << reply.error().name();\
    return;\
    }\
    }

#define UNMASK_SERVICE(service) \
{\
    auto reply = m_systemd1ManagerInter->UnmaskUnitFiles(QStringList() << service, true);\
    if (reply.isError()) {\
    qWarning() << "failed to unmask service: " << service << ", error: " << reply.error().name();\
    return;\
    }\
    }

#define START_SERVICE(service) \
{\
    auto reply = m_systemd1ManagerInter->StartUnit(service, "replace");\
    if (reply.isError()) {\
    qWarning() << "failed to start service: " << service << ", error: " << reply.error().name();\
    return;\
    }\
    }

#define STOP_SERVICE(service) \
{\
    auto reply = m_systemd1ManagerInter->StopUnit(service, "replace");\
    if (reply.isError()) {\
    qWarning() << "failed to stop service: " << service << ", error: " << reply.error().name();\
    return;\
    }\
    }

#define VIEW_SERVICE(service) \
{\
    QDBusPendingReply<QString> reply = m_systemd1ManagerInter->GetUnitFileState(service);\
    if (reply.isError()) {\
    qWarning() << "failed to get service state: " << service << ", error: " << reply.error().name();\
    return;\
    }\
    qDebug() << service << " state: " << reply.value();\
    }\

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , m_locked(false)
    , m_currentUid(QString::number(getuid()))
    , m_stage(0)
    , m_currentSessionPath("/")
    , m_powerInter(new org::deepin::daemon::PowerManager1("org.deepin.daemon.PowerManager", "/org/deepin/daemon/PowerManager", QDBusConnection::systemBus(), this))
    , m_audioInter(new org::deepin::daemon::Audio1("org.deepin.daemon.Audio1", "/org/deepin/daemon/Audio1", QDBusConnection::sessionBus(), this))
    , m_login1ManagerInter(new org::freedesktop::login1::Manager("org.freedesktop.login1", "/org/freedesktop/login1", QDBusConnection::systemBus(), this))
    , m_login1UserInter(new org::freedesktop::login1::User("org.freedesktop.login1", m_login1ManagerInter->GetUser(getuid()).value().path(), QDBusConnection::systemBus(), this))
    , m_login1SessionInter(new org::freedesktop::login1::Session("org.freedesktop.login1", m_login1UserInter->display().path.path(), QDBusConnection::systemBus(), this))
    , m_systemd1ManagerInter(new org::freedesktop::systemd1::Manager("org.freedesktop.systemd1", "/org/freedesktop/systemd1", QDBusConnection::sessionBus(), this))
    , m_DBusInter(new org::freedesktop::DBus("org.freedesktop.DBus", "/org/freedesktop/DBus", QDBusConnection::sessionBus(), this))
{
    initConnections();

    // 处理异常退出的情况
    handleOSSignal();
}

void SessionManager::initConnections()
{
    connect(m_login1SessionInter, &org::freedesktop::login1::Session::ActiveChanged, [=](bool active) {
        qDebug() << "session active status changed to:" << active;
        if (active) {
            // 变活跃
        } else {
            // 变不活跃
            bool isPreparingForSleep = m_login1ManagerInter->preparingForSleep();
            if (!isPreparingForSleep) {
                qDebug() << "call objLoginSessionSelf.Lock";
                m_login1SessionInter->Lock();
            }
        }
    });

    if (Dconf::GetValue("com.deepin.dde.startdde", "", "swap-sched-enabled", QVariant(false)).toBool()) {
        initSwapSched();
    }

    connect(m_DBusInter, &org::freedesktop::DBus::NameOwnerChanged,
            [ = ](const QString &name, const QString &oldOwner, const QString &newOwner) {
        if (newOwner == "" && oldOwner != "" && name == oldOwner && name.startsWith(":")) {
            // uniq name lost
        }
    });

    // TODO qt不支持signal和method同名，可以采用直接关联的方式完成目的
    connect(m_login1SessionInter, SIGNAL(Lock()), this, SLOT(handleLoginSessionLocked()));
    connect(m_login1SessionInter, SIGNAL(Unlock()), this, SLOT(handleLoginSessionUnlocked()));
}

void SessionManager::initSwapSched()
{
    qDebug() << "init swap sched";
    // TODO
}

void SessionManager::prepareLogout(bool force)
{
    if (!force)
        launchAutostopScripts();

    stopSogouIme();
    stopLangSelector();
    stopAtSpiService();
    stopBAMFDaemon();
    stopRedshift();
    stopObexService();

    // 防止注销时，蓝牙音频设置没有断开连接
    disconnectAudioDevices();

    // 强制注销时不播放音乐
    if (!force && canPlayEvent("desktop-logout")) {
        playLogoutSound();
    } else {
        stopPulseAudioService();
    }
}

void SessionManager::doLogout()
{
#ifndef QT_DEBUG
    QDBusPendingReply<> reply = m_login1SessionInter->Terminate();
    if (reply.isError()) {
        qWarning() << "failed to terminate session self";
    }

    qApp->quit();
#else
    qInfo() << "logout completed, bug not now";
#endif
}

SessionManager *SessionManager::instance()
{
    static SessionManager instance;
    return &instance;
}

bool SessionManager::locked()
{
    return m_locked;
}

QString SessionManager::currentUid()
{
    return m_currentUid;
}

void SessionManager::setCurrentUid(QString uid)
{
    m_currentUid = uid;
}

int SessionManager::stage()
{
    return m_stage;
}

void SessionManager::setStage(int value)
{
    // TODO 待确定此属性的用途
    m_stage = value;
}

QDBusObjectPath SessionManager::currentSessionPath()
{
    m_currentSessionPath = m_login1UserInter->display().path;
    if (m_currentSessionPath.path().isEmpty()) {
        // 要保证 CurrentSessionPath 属性值的合法性
        m_currentSessionPath.setPath("/");
    }

    return m_currentSessionPath;
}

void SessionManager::setCurrentSessionPath(QDBusObjectPath value)
{
    m_currentSessionPath = value;
}

bool SessionManager::AllowSessionDaemonRun()
{
    // TODO: run com.deepin.daemon.Daemon.StartPart2
    return true;
}

bool SessionManager::CanHibernate()
{
    if (QString(getenv("POWER_CAN_SLEEP")) == "0") {
        return false;
    }

    // 是否支持休眠
    if (!m_powerInter->CanHibernate()) {
        return false;
    }

    // 当前能否休眠
    QString canHibernate = m_login1ManagerInter->CanHibernate();
    return canHibernate == "yes";
}

bool SessionManager::CanLogout()
{
    return true;
}

bool SessionManager::CanReboot()
{
    QString canReboot = m_login1ManagerInter->CanReboot();
    return canReboot == "yes";
}

bool SessionManager::CanShutdown()
{
    QString canPowerOff = m_login1ManagerInter->CanPowerOff();
    return canPowerOff == "yes";
}

bool SessionManager::CanSuspend()
{
    if (QString(getenv("POWER_CAN_SLEEP")) == "0") {
        return false;
    }

    // 是否支持待机
    if (!m_powerInter->CanSuspend()) {
        return false;
    }

    // 当前能否待机
    QString canSuspend = m_login1ManagerInter->CanSuspend();
    return canSuspend == "yes";
}

void SessionManager::ForceLogout()
{
    qDebug() << "force logout";

    prepareLogout(true);
    clearCurrentTty();
    doLogout();
}

void SessionManager::ForceReboot()
{
    reboot(true);
}

void SessionManager::ForceShutdown()
{
    shutdown(true);
}

Q_DECL_DEPRECATED void SessionManager::Logout()
{
    Q_UNUSED(void());
}

Q_DECL_DEPRECATED void SessionManager::PowerOffChoose()
{
    Q_UNUSED(void());
}

Q_DECL_DEPRECATED void SessionManager::Reboot()
{
    Q_UNUSED(void())
}

bool SessionManager::Register(const QString &id)
{
    Q_UNUSED(id)
    return false;
}

void SessionManager::RequestHibernate()
{
    QDBusPendingReply<> reply = m_login1ManagerInter->Hibernate(false);
    if (reply.isError()) {
        qWarning() << "failed to hibernate, error: " << reply.error().name();
    }

    if (Utils::SettingValue("com.deepin.dde.startdde", QByteArray(), "quick-black-screen", false).toBool()) {
        setDPMSMode(false);
    }
}

void SessionManager::RequestLock()
{
    QDBusInterface inter("com.deepin.dde.lockFront", "/com/deepin/dde/lockFront", "com.deepin.dde.lockFront", QDBusConnection::sessionBus(), this);
    const QDBusMessage &msg = inter.call("Show");
    if (!msg.errorName().isEmpty()) {
        qWarning() << "failed to lock, error: " << msg.errorMessage();
    }
}

void SessionManager::RequestLogout()
{
    qDebug() << "request logout";

    prepareLogout(false);
    clearCurrentTty();
    doLogout();
}

void SessionManager::RequestReboot()
{
    reboot(false);
}

void SessionManager::RequestShutdown()
{
    shutdown(false);
}

void SessionManager::RequestSuspend()
{
    if (QFile("/etc/deepin/no_suspend").exists()) {
        QThread::sleep(1);
        setDPMSMode(false);
        return;
    }

    // 使用窗管接口进行黑屏处理
    if (Utils::SettingValue("com.deepin.dde.startdde", QByteArray(), "quick-black-screen", false).toBool()) {
        QDBusInterface inter("org.kde.KWin", "/BlackScreen", "org.kde.kwin.BlackScreen", QDBusConnection::sessionBus(), this);
        const QDBusMessage &msg = inter.call("setActive", true);
        if (!msg.errorName().isEmpty())
            qWarning() << "failed to create blackscreen, error: " << msg.errorName();
    }

    QDBusPendingReply<> reply = m_login1ManagerInter->Suspend(false);
    if (reply.isError()) {
        qWarning() << "failed to suspend, error:" << reply.error().name();
    }
}

void SessionManager::SetLocked(bool lock)
{
    // 仅允许dde-lock进程调用
    QString cmdLine = QString("/proc/%1/cmdline").arg(connection().interface()->servicePid(message().service()));
    QFile file(cmdLine);

    // NOTE: 如果以deepin-turbo进行加速启动，这里是不准确的，可能需要判断desktop文件的全路径，不过deepin-turbo后续应该会放弃支持
    if (!file.open(QIODevice::ReadOnly) || !file.readAll().startsWith("/usr/bin/dde-lock")) {
        qWarning() << "failed to get caller infomation or caller is illegal.";
        return;
    }

    m_locked = lock;
    Q_EMIT lockedChanged(m_locked);
}

Q_DECL_DEPRECATED void SessionManager::Shutdown()
{
    Q_UNUSED(void())
}

/**
 * @brief SessionManager::ToggleDebug
 * @note 允许在运行时打开debug日志输出，使得不用重启即可获取部分重要日志
 */
void SessionManager::ToggleDebug()
{
    QLoggingCategory::installFilter([] (QLoggingCategory *category) {
        category->setEnabled(QtDebugMsg, true);
    });
}

void SessionManager::prepareShutdown(bool force)
{
    stopSogouIme();
    stopBAMFDaemon();

    if (!force)
        preparePlayShutdownSound();

    stopPulseAudioService();
}

void SessionManager::clearCurrentTty()
{
    QDBusInterface inter("org.deepin.daemon.Daemon1", "/org/deepin/daemon/Daemon1", "org.deepin.daemon.Daemon1", QDBusConnection::systemBus(), this);
    const QDBusMessage &msg = inter.call("ClearTty", m_login1SessionInter->vTNr());
    if (!msg.errorMessage().isEmpty())
        qWarning() << "failed to clear tty, error: " << msg.errorMessage();
}

void SessionManager::init()
{
    qInfo() << "session manager init";
    // TODO recover

    startAtSpiService();
    startObexService();

    qInfo() << "session manager init finished";
}

void SessionManager::stopSogouIme()
{
    // TODO 使用kill函数杀死进程，前提是进程存在
    EXEC_COMMAND("pkill"
                 , QStringList()
                 << "-ef"
                 << "sogouImeService");
}

void SessionManager::stopLangSelector()
{
    // TODO 待优化，可以跟随当前用户一同退出
    EXEC_COMMAND("pkill"
                 , QStringList()
                 << "-ef"
                 << "-u"
                 << QString::number(getuid()) << "/usr/lib/deepin-daemon/langselector"
                 );
}


void SessionManager::launchAutostopScripts()
{
    QStringList paths;
    paths.append(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()+ QDir::separator() + "autostop");
    paths.append("/etc/xdg/autostop");

    for (auto path : paths) {
        QDir scriptDir(path);
        const QFileInfoList &fileInfos = scriptDir.entryInfoList(QDir::Files);
        for (auto info : fileInfos) {
            qDebug() << "[autostop] script will be launched: " << info.absoluteFilePath();
            EXEC_COMMAND("/bin/bash", QStringList() << "-c" << info.absoluteFilePath());
        }
    }
}

void SessionManager::startAtSpiService()
{
    UNMASK_SERVICE(AT_SPI_SERVICE);
    START_SERVICE(AT_SPI_SERVICE);
    VIEW_SERVICE(AT_SPI_SERVICE);
}

void SessionManager::stopAtSpiService()
{
    MASK_SERVICE(AT_SPI_SERVICE);
    VIEW_SERVICE(AT_SPI_SERVICE);
}

void SessionManager::startObexService()
{
    UNMASK_SERVICE(OBEX_SERVICE);
    START_SERVICE(OBEX_SERVICE);
    VIEW_SERVICE(OBEX_SERVICE);
}

void SessionManager::stopObexService()
{
    MASK_SERVICE(OBEX_SERVICE);
    VIEW_SERVICE(OBEX_SERVICE);
}

void SessionManager::stopPulseAudioService()
{
    auto msg = QDBusInterface("org.deepin.daemon.Audio1"
                              , "/org/deepin/daemon/Audio1"
                              , "org.deepin.daemon.Audio1"
                              , QDBusConnection::sessionBus(), this)
            .call("NoRestartPulseAudio");
    if (!msg.errorMessage().isEmpty())
        qWarning() << "error: " << msg.errorMessage();

    STOP_SERVICE(PULSEAUDIO_SERVICE);
    VIEW_SERVICE(PULSEAUDIO_SERVICE);
}

void SessionManager::stopBAMFDaemon()
{
    STOP_SERVICE(BAMFDAEMON_SERVICE);
    VIEW_SERVICE(BAMFDAEMON_SERVICE);
}

void SessionManager::stopRedshift()
{
    STOP_SERVICE(REDSHIFT_SERVICE);
    VIEW_SERVICE(REDSHIFT_SERVICE);
}

void SessionManager::disconnectAudioDevices()
{
    auto msg = QDBusInterface("org.deepin.system.Bluetooth1"
                              , "/org/deepin/system/Bluetooth1"
                              , "org.deepin.system.Bluetooth1"
                              , QDBusConnection::systemBus(), this)
            .call("DisconnectAudioDevices");
    if (!msg.errorMessage().isEmpty()) {
        qWarning() << "error: " << msg.errorMessage();
    } else {
        qDebug() << "success to disconnect audio devices";
    }
}

// 准备好关机时的音频文件
void SessionManager::preparePlayShutdownSound()
{
    // TODO
}

bool SessionManager::canPlayEvent(const QString &event)
{
    if (event == SOUND_EFFECT_ENABLED_KEY || event == SOUND_EFFECT_PLAYER_KEY)
        return false;

    return Utils::SettingValue(SOUND_EFFECT_SCHEMA, QByteArray(), SOUND_EFFECT_ENABLED_KEY, false).toBool();
}

void SessionManager::playLogoutSound()
{
    do {
        auto sink = m_audioInter->defaultSink();
        if (sink.path().isEmpty()) {
            qDebug() << "no default sink";
            break;
        }

        auto sinkInter = new org::deepin::daemon::audio1::Sink(
                    "org.deepin.daemon.Audio1"
                    , sink.path()
                    , QDBusConnection::sessionBus()
                    , this);
        if (sinkInter->mute()) {
            qDebug() << "default sink is mute";
            break;
        }
    } while(0);

    stopPulseAudioService();

    auto soundTheme = Utils::SettingValue(APPEARANCE_SCHEMA, QByteArray(), APPEARANCE_SOUND_THEME_KEY, QString()).toString();
    com::deepin::api::SoundThemePlayer inter("com.deepin.api.SoundThemePlayer", "/com/deepin/api/SoundThemePlayer", QDBusConnection::systemBus(), this);
    // TODO device?
    QDBusPendingReply<> reply = inter.Play(soundTheme, "desktop-logout", "");
    if (reply.isError()) {
        qWarning() << "failed to play logout sound, error: " << reply.error().name();
    }
}

void SessionManager::setDPMSMode(bool on)
{
    if (Utils::IS_WAYLAND_DISPLAY) {
        EXEC_COMMAND("dde_wldpms", QStringList() << "-s" << (on ? "On" : "Off"));
    } else {
        // TODO 可通过Xlib发送对应请求实现此功能
        EXEC_COMMAND("xset", QStringList() << "dpms" << "force" << (on ? "on" : "off"));
    }
}

/**
 * @brief sig_crash 意外退出时处理一些事情
 * @param sig 收到的异常退出信号类型
 */
[[noreturn]] void sig_crash(int sig) {
    Q_UNUSED(sig);
    SessionManager::instance()->doLogout();
    exit(-1);
};

void SessionManager::handleOSSignal()
{
    signal(SIGINT,  sig_crash);
    signal(SIGABRT, sig_crash);
    signal(SIGTERM, sig_crash);
    signal(SIGSEGV, sig_crash);
}

void SessionManager::shutdown(bool force)
{
    prepareShutdown(force);
    clearCurrentTty();

    QDBusPendingReply<> reply = m_login1ManagerInter->PowerOff(false);
    if (reply.isError()) {
        qWarning() << "failed to power off, error: " << reply.error().name();
    }

    if (Utils::SettingValue("com.deepin.dde.startdde", QByteArray(), "quick-black-screen", false).toBool()) {
        setDPMSMode(false);
    }

    QDBusPendingReply<> reply2 = m_login1SessionInter->Terminate();
    if (reply2.isError()) {
        qWarning() << "self session failed to terminate, error: " << reply2.error().name();
    }

    qApp->quit();
}

void SessionManager::reboot(bool force)
{
    prepareShutdown(force);
    clearCurrentTty();

    QDBusPendingReply<> reply = m_login1ManagerInter->Reboot(false);
    if (reply.isError()) {
        qWarning() << "failed to reboot, error: " << reply.error().name();
    }

    if (Utils::SettingValue("com.deepin.dde.startdde", QByteArray(), "quick-black-screen", false).toBool()) {
        setDPMSMode(false);
    }

    QDBusPendingReply<> reply2 = m_login1SessionInter->Terminate();
    if (reply2.isError()) {
        qWarning() << "self session failed to terminate, error: " << reply2.error().name();
    }

    qApp->quit();
}

void SessionManager::handleLoginSessionLocked()
{
    qDebug() << "login session locked.";
    // 在特殊情况下，比如用 dde-switchtogreeter 命令切换到 greeter, 即切换到其他 tty, RequestLock 方法不能立即返回。

    // 如果已经锁定，则立即返回
    if (locked()) {
        qDebug() << "already locked";
        return;
    }

    RequestLock();
}

void SessionManager::handleLoginSessionUnlocked()
{
    qDebug() << "login session unlocked.";

    bool sessionActive = m_login1SessionInter->active();
    if (sessionActive) {
        Q_EMIT Unlock();
    } else {
        // 图形界面 session 不活跃，此时采用 kill 掉锁屏前端的特殊操作，
        // 如果不 kill 掉它，则一旦 session 变活跃，锁屏前端会在很近的时间内执行锁屏和解锁，
        // 会使用系统通知报告锁屏失败。 而在此种特殊情况下 kill 掉它，并不会造成明显问题。
        do {
            org::freedesktop::DBus dbusInter("org.freedesktop.DBus", "/org/freedesktop/DBus", QDBusConnection::sessionBus());
            QDBusPendingReply<QString> ownerReply = dbusInter.GetNameOwner("com.deepin.dde.lockFront");
            if (ownerReply.isError()) {
                qWarning() << "failed to get lockFront service owner";
                return;
            }
            const QString &owner = ownerReply.value();

            QDBusPendingReply<uint> pidReply = dbusInter.GetConnectionUnixProcessID(owner);
            if (pidReply.isError()) {
                qWarning() << "failed to get lockFront service pid";
                return;
            }
            uint pid = pidReply.value();

            // 默认发送 SIGTERM, 礼貌的退出信号
            EXEC_COMMAND("kill", QStringList() << QString::number(pid));

            Q_EMIT Unlock();
        } while (false);

        m_locked = false;
        Q_EMIT lockedChanged(false);
    }
}

