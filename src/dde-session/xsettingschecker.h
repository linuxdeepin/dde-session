#ifndef XSETTINGS_CHECKER_H
#define XSETTINGS_CHECKER_H
#include "com_deepin_XSettings.h"

#include <QObject>

/**
 * @brief The XSettingsChecker class
 * @brief 初始化系统字体和主题相关配置
 */
class XSettingsChecker : public QObject
{
public:
    explicit XSettingsChecker(QObject *parent = nullptr);

    void init();

private:
    void initXSettingsFont();
    void initQtTheme();
    void initLeftPtrCursor();

    void loadDefaultFontConfig(QString &defaultFont, QString &defaultMonoFont);

private:
    com::deepin::XSettings *m_xSettingsInter;
};

#endif // XSETTINGS_CHECKER_H
