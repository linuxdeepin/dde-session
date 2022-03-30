#include "session.h"

#include <QCoreApplication>

Session::Session(QObject *parent)
    : QObject{parent}
{

}

void Session::Logout()
{
    qApp->quit();
}
