// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONPATHLIST_H
#define SESSIONPATHLIST_H

#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QList>

#include "sessionpath.h"

typedef QList<SessionPath> SessionPathList;

inline bool operator !=(const SessionPathList &list1, const SessionPathList &list2)
{
    return list1.size() != list2.size() || list1 != list2;
}

Q_DECLARE_METATYPE(SessionPathList)

void registerSessionPathListMetaType();

#endif // SESSIONPATHLIST_H
