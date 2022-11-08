#include <QCoreApplication>

#include "loginremindermanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-login-reminder");

    LoginReminderManager m;
    m.init();
    return app.exec();
}
