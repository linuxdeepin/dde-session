#ifndef SESSION_H
#define SESSION_H

#include <QObject>

class Session : public QObject
{
    Q_OBJECT
public:
    explicit Session(QObject *parent = nullptr);

public Q_SLOTS:
    void Logout();
    int GetSessionPid();
    QString GetSessionPath();

    void setSessionPid(int pid);
    void setSessionPath();

private:
    int m_sessionPid;
    QString m_selfSessionPath;
};

#endif // SESSION_H
