#ifndef LOGINREMINDERMANAGER_H
#define LOGINREMINDERMANAGER_H

#include <QObject>
#include <QDBusMessage>

class LoginReminderManager : public QObject
{
    Q_OBJECT
public:
    explicit LoginReminderManager(QObject *parent = nullptr);

    void init();

public Q_SLOTS:
    void handleActionInvoked(uint id, QString action);
    void handleNotificationClosed(uint id, uint);

private:
    uint m_notifyId;
    QString m_title;
    QString m_content;
};

#endif // LOGINREMINDERMANAGER_H
