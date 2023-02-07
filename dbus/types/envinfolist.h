// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDBusMetaType>

typedef QMap<QString, QString> EnvInfoList;

void registerEnvInfoListMetaType();
