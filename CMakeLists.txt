cmake_minimum_required(VERSION 3.13)

project(dde-session LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

add_subdirectory("src")
add_subdirectory("systemd")

install(FILES dbus/org.deepin.Session.service
    DESTINATION share/dbus-1/services/)
