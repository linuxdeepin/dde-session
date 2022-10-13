#pragma once

#include <QDBusMetaType>

typedef QMap<QString, QString> EnvInfoList;

void registerEnvInfoListMetaType();
