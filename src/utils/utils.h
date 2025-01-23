// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <QDebug>

// system services
#define AT_SPI_SERVICE "at-spi-dbus-bus.service"
#define OBEX_SERVICE "obex.service"
#define PULSEAUDIO_SERVICE "pulseaudio.service"
#define PIPEWIRE_SOCKET "pipewire.socket"
#define PIPEWIRE_PULSE_SOCKET "pipewire-pulse.socket"
#define BAMFDAEMON_SERVICE "bamfdaemon.service"
#define REDSHIFT_SERVICE "redshift.service"
#define DDE_DOCK_SERVICE "dde-dock.service"

// sound
#define SOUND_EFFECT_SCHEMA "com.deepin.dde.sound-effect"
#define SOUND_EFFECT_ENABLED_KEY "enabled"
#define SOUND_EFFECT_PLAYER_KEY "player"

// macro funtions
#define EXEC_COMMAND(cmd, args) \
    QProcess p;\
    p.start(cmd, args);\
    p.waitForFinished(-1);\
    if (p.exitCode() != 0) {\
    qWarning() << "failed to run: " << cmd << ", args:" << args;\
    }

namespace Utils {

const bool IS_WAYLAND_DISPLAY = qgetenv("XDG_SESSION_TYPE").startsWith("wayland");
}

#endif // UTILS_H
