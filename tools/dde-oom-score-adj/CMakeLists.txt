# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

set(BIN_NAME dde-oom-score-adj)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CAPNG REQUIRED IMPORTED_TARGET libcap-ng)

add_executable(${BIN_NAME}
    main.cpp
    oomscoreadjuster.h
    oomscoreadjuster.cpp
)

target_link_libraries(${BIN_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Dtk${DTK_VERSION_MAJOR}::Core
    PkgConfig::CAPNG
)

install(TARGETS ${BIN_NAME} DESTINATION bin)
install(FILES dde-oom-score-adj.desktop DESTINATION /etc/xdg/autostart/)
