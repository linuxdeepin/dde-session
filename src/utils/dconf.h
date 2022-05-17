/*
 * Copyright (C) 2022 ~ 2023 Deepin Technology Co., Ltd.
 *
 * Author:     weizhixiang <weizhixiang@uniontech.com>
 *
 * Maintainer: weizhixiang <weizhixiang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DCONF_H
#define DCONF_H

#include <DConfig>

#include <QObject>
#include <QString>

DCORE_USE_NAMESPACE

// Dconfig 配置类
class Dconf
{
public:
    Dconf();
    ~Dconf();

public:
    static const QVariant GetValue(const QString &name, const QString &subPath = QString(), const QString &key = QString(), const QVariant &fallback = QVariant());
    static bool SetValue(const QString &name, const QString &subPath, const QString &key, const QVariant &value);

private:
    static DConfig *connectDconf(const QString &name, const QString &subpath = QString(), QObject *parent = nullptr);
};

#endif // DCONF_H