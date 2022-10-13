#pragma once

#include <QString>

class EnvHandler {
public:
    static bool EnvInit(QString sessionType);

    static bool x11EnvInit();
    static bool waylandEnvInit();
    static bool setEnv(QString env, QString value);
    static bool unsetEnv(QString env);
};