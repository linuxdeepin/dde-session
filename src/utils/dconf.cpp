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

#include "utils/dconf.h"

#include <QSharedPointer>
#include <QDebug>

Dconf::Dconf()
{

}

Dconf::~Dconf()
{

}

DConfig *Dconf::connectDconf(const QString &name, const QString &subpath, QObject *parent)
{
    DConfig *config = DConfig::create("dde-session", name, subpath, parent);
    if (config == nullptr) {
        qDebug() << "Cannot find dconfigs, name:" << name;
        return nullptr;
    }
    if (!config->isValid()) {
        qDebug() << "Cannot find dconfigs, name:" << name;
        delete config;
        return nullptr;
    }

    return config;
}

const QVariant Dconf::GetValue(const QString &name, const QString &subPath, const QString &key, const QVariant &fallback)
{
    QSharedPointer<DConfig> config(connectDconf(name, subPath));
    if (config && config->isValid() && config->keyList().contains(key)) {
        QVariant v = config->value(key);
        return v;
    }

    qDebug() << "Cannot find dconfigs, name:" << name
             << " subPath:" << subPath << " key:" << key
             << "Use fallback value:" << fallback;
    return fallback;
}

bool Dconf::SetValue(const QString &name, const QString &subPath, const QString &key, const QVariant &value)
{
    QSharedPointer<DConfig> config(connectDconf(name, subPath));
    if (config && config->isValid() && config->keyList().contains(key)) {
        config->setValue(key, value);
        return true;
    }

    qDebug() << "Cannot find dconfigs, name:" << name
             << " subPath:" << subPath << " key:" << key;
    return false;
}