#include "xsettingschecker.h"
#include "utils.h"

#include <QDBusConnection>
#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QJsonParseError>
#include <QJsonObject>

XSettingsChecker::XSettingsChecker(QObject *parent)
    : QObject(parent) // TODO org.deepin.dde.SessionManager1 -> org.deepin.dde.XSettings1
    , m_xSettingsInter(new QDBusInterface("org.deepin.dde.SessionManager1", "/org/deepin/dde/XSettings1", "org.deepin.dde.XSettings1", QDBusConnection::sessionBus(), this))
{

}

void XSettingsChecker::init()
{
    qDebug() << "xsettings checker";

    initXSettingsFont();
    initQtTheme();
    initLeftPtrCursor();

    qDebug() << "xsettings checker finished";
}

void XSettingsChecker::initXSettingsFont()
{
    QString defaultFont;
    QString defaultMonoFont;
    loadDefaultFontConfig(defaultFont, defaultMonoFont);

    // check xs font name
    {
        QDBusPendingReply<QString> reply = GetXSettingsString("Qt/FontName");
        if (reply.isError()) {
            qWarning() << "failed to get font name from xsettings service, error: " << reply.error().name();
        } else {
            const QString &xSettingsFontName = reply.value();
            if (xSettingsFontName.isEmpty()) {
                reply = SetXSettingsString("Qt/FontName", defaultFont);
                if (reply.isError()) {
                    qWarning() << "failed to set font name to xsettings service, error: " << reply.error().name();
                }
                qDebug() << "set qt font to xsettings service: " << defaultFont;
            }
        }
    }

    // check xs mono font name
    {
        QDBusPendingReply<QString> reply = GetXSettingsString("Qt/MonoFontName");
        if (reply.isError()) {
            qWarning() << "failed to get mono font name from xsettings service, error: " << reply.error().name();
        } else {
            const QString &xSettingsMonoFontName = reply.value();
            if (xSettingsMonoFontName.isEmpty()) {
                reply = SetXSettingsString("Qt/MonoFontName", defaultMonoFont);
                if (reply.isError()) {
                    qWarning() << "failed to set mono font name to xsettings service, error: " << reply.error().name();
                } else {
                    qDebug() << "set qt mono font to xsettings service: " << defaultMonoFont;
                }
            }
        }
    }
}

void XSettingsChecker::initQtTheme()
{
    const QString &qtThemeConf = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()
            + QDir::separator()
            + "deepin/qt-theme.ini";

    QSettings settings(qtThemeConf, QSettings::IniFormat);
    settings.beginGroup("Theme");

    // icon-theme
    QString iconTheme = settings.value("IconThemeName").toString();
    if (iconTheme.isEmpty()) {
        iconTheme = Utils::SettingValue("com.deepin.dde.appearance", QByteArray(), "icon-theme", QString()).toString();
        settings.setValue("IconThemeName", iconTheme);
        qDebug() << "set icon theme name in qt theme: " << iconTheme;
    }

    // font size
    double fontSize = settings.value("FontSize").toDouble();
    if (fontSize - 0 < (1e-6)) {
        fontSize = Utils::SettingValue("com.deepin.dde.appearance", QByteArray(), "font-size", 0.0).toDouble();
        settings.setValue("FontSize", fontSize);
        qDebug() << "set font size in qt theme: " << fontSize;
    }

    // font
    QString fontName = settings.value("Font").toString();
    if (fontName.isEmpty()) {
        QString unused;
        loadDefaultFontConfig(fontName, unused);
        settings.setValue("Font", fontName);
        qDebug() << "set font name in qt theme: " << fontName;
    }

    // mono font
    QString monoFontName = settings.value("MonFont").toString();
    if (monoFontName.isEmpty()) {
        QString unused;
        loadDefaultFontConfig(unused, monoFontName);
        settings.setValue("MonFont", monoFontName);
        qDebug() << "set mono font name in qt theme: " << monoFontName;
    }
}

void XSettingsChecker::initLeftPtrCursor()
{
    // TODO 这部分原有逻辑和greeter中部分操作重合了，应该不需要设置
    // 如果登录成功后光标显示异常，这里再处理，或者把greeter的处理看流程迁移过来
}

void XSettingsChecker::loadDefaultFontConfig(QString &defaultFont, QString &defaultMonoFont)
{
    // 从配置文件中获取信息
    QFile fontFile("/usr/share/deepin-default-settings/fontconfig.json");
    if (!fontFile.exists() || !fontFile.open(QIODevice::ReadOnly)) {
        qWarning() << "font config not exists, maybe we found an error in `deepin-default-settings`";
        defaultFont = "Noto Sans";
        defaultMonoFont = "Noto Mono";
        return;
    }

    // 解析配置文件
    QJsonParseError jsonError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(fontFile.readAll(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        qWarning() << "failed to parse font config, error type: " << jsonError.error;
        defaultFont = "Noto Sans";
        defaultMonoFont = "Noto Mono";
        return;
    }

    // 获取当前用户语言的默认字体信息
    QJsonObject rootObj = jsonDocument.object();
    QJsonValue rootValue = rootObj.value(QLocale().name());
    QJsonObject localeObj = rootValue.toObject();
    defaultFont = localeObj.value("Standard").toString();
    defaultMonoFont = localeObj.value("Mono").toString();
    if (defaultFont.isEmpty())
        defaultFont = "Noto Sans";
    if (defaultMonoFont.isEmpty())
        defaultMonoFont = "Noto Mono";

    qDebug() << "default font: " << defaultFont
             << ", default mono font: " << defaultMonoFont;
}

QDBusPendingReply<QString> XSettingsChecker::GetXSettingsString(const QString &prop)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(prop);
    return m_xSettingsInter->asyncCallWithArgumentList(QStringLiteral("GetString"), argumentList);
}

QDBusPendingReply<> XSettingsChecker::SetXSettingsString(const QString &prop, const QString &value)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(prop) << QVariant::fromValue(value);
    return m_xSettingsInter->asyncCallWithArgumentList(QStringLiteral("SetString"), argumentList);
}
