# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
qt_add_dbus_interface(DBUS_SOURCES
    ../../dbus/adaptor/org.deepin.dde.Session1.xml
    Session
)

add_executable(dde-session-ctl
    main.cpp
    ${DBUS_SOURCES}
    ../../dbus/adaptor/org.deepin.dde.Session1.xml
)
target_link_libraries(dde-session-ctl
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
)

install(TARGETS dde-session-ctl DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
