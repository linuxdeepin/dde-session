
#include "envinfolist.h"

void registerEnvInfoListMetaType()
{
    qRegisterMetaType<EnvInfoList>("EnvInfoList");
    qDBusRegisterMetaType<EnvInfoList>();
}
