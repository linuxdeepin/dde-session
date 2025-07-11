# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Tools)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set (CMAKE_CXX_STANDARD 11)

# qdbusxml2cpp-fix
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/dbusinterface/)
set(QT_DBUS_INTERFACE_INCLUDE ${PROJECT_BINARY_DIR}/dbusinterface/)

macro(qt_add_dbus_interface_fix srcs xml class file)
  execute_process(COMMAND ${DTK_XML2CPP} -c ${class} -p ${PROJECT_BINARY_DIR}/dbusinterface/${file} ${PROJECT_SOURCE_DIR}/dbus/interface/${xml})
  list(APPEND ${srcs}
    ${PROJECT_BINARY_DIR}/dbusinterface/${file}.h
    ${PROJECT_BINARY_DIR}/dbusinterface/${file}.cpp
  )
endmacro(qt_add_dbus_interface_fix)

# qtdbusextended
set(DBUS_INCLUDE ${PROJECT_SOURCE_DIR}/dbus/)
list(APPEND DBUSEXTENDED_SOURCE
  ${PROJECT_SOURCE_DIR}/src/qtdbusextended/DBusExtended
  ${PROJECT_SOURCE_DIR}/src/qtdbusextended/dbusextended.h
  ${PROJECT_SOURCE_DIR}/src/qtdbusextended/DBusExtendedAbstractInterface
  ${PROJECT_SOURCE_DIR}/src/qtdbusextended/dbusextendedabstractinterface.h
  ${PROJECT_SOURCE_DIR}/src/qtdbusextended/dbusextendedpendingcallwatcher_p.h
  ${PROJECT_SOURCE_DIR}/src/qtdbusextended/dbusextendedabstractinterface.cpp
  ${PROJECT_SOURCE_DIR}/src/qtdbusextended/dbusextendedpendingcallwatcher.cpp
)

add_subdirectory("dde-session-ctl")
add_subdirectory("dde-session")
