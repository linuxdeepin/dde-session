# SPDX-FileCopyrightText: 2021 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(BIN_NAME dde-quick-login)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCES
    main.cpp
    lockprocessmanager.cpp
)

set(HEADERS
    lockprocessmanager.h
)

add_executable(${BIN_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${BIN_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Dtk${DTK_VERSION_MAJOR}::Core
    PkgConfig::Systemd
)

install(TARGETS ${BIN_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
