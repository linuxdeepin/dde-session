find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Concurrent DBus REQUIRED)
find_package(DtkCore REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)

# dbus adaptor
qt5_add_dbus_adaptor(ADAPTER_SOURCES
                     ../../dbus/adaptor/org.deepin.Session.xml
                     impl/session.h
                     Session)
qt5_add_dbus_adaptor(ADAPTER_SOURCES
                     ../../dbus/adaptor/com.deepin.SessionManager.xml
                     impl/sessionmanager.h
                     SessionManager)

# dbus interface
qt5_add_dbus_interface_fix(INTERFACE_SOURCES com.deepin.daemon.PowerManager.xml PowerManager com_deepin_PowerManager)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES com.deepin.system.Bluetooth.xml Bluetooth com_deepin_system_Bluetooth)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES com.deepin.daemon.Daemon.xml SessionDaemon com_deepin_daemon_Daemon)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.login1.Manager.xml Login1Manager org_freedesktop_login1_Manager)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.login1.Session.xml Login1Session org_freedesktop_login1_Session)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.DBus.xml DBus org_freedesktop_DBus)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.systemd1.Manager.xml Systemd1Manager org_freedesktop_systemd1_Manager)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.systemd1.Job.xml Systemd1Job org_freedesktop_systemd1_Job)

message("DBUSEXTENDED_SOURCE", ${DBUSEXTENDED_SOURCE})
message("INTERFACE_SOURCES", ${INTERFACE_SOURCES})
message("ADAPTER_SOURCES", ${ADAPTER_SOURCES})

add_executable(dde-session
  main.cpp
  impl/session.h
  impl/session.cpp
  impl/sessionmanager.h
  impl/sessionmanager.cpp
  ../utils/dconf.h
  ../utils/dconf.cpp
  ../utils/fifo.h
  ../utils/fifo.cpp
  ${ADAPTER_SOURCES}
  ${INTERFACE_SOURCES}
  ${DBUSEXTENDED_SOURCE}
)


target_link_libraries(dde-session
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Concurrent
    PkgConfig::Systemd
    ${DtkCore_LIBRARIES}
)

target_include_directories(dde-session PUBLIC
    ../
    ${QT_DBUS_INTERFACE_INCLUDE}
    ${DBUSEXTENDED_INCLUDE}
)

install(TARGETS dde-session DESTINATION bin)
