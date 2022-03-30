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
};

#endif // SESSION_H
