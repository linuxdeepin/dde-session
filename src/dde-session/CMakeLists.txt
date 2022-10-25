find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Concurrent DBus REQUIRED)
find_package(DtkCore REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)
pkg_check_modules(QGSettings REQUIRED IMPORTED_TARGET gsettings-qt)
pkg_check_modules(GLIB2 REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(SECRET1 REQUIRED IMPORTED_TARGET libsecret-1)

# dbus adaptor
qt5_add_dbus_adaptor(ADAPTER_SOURCES
    ../../dbus/adaptor/org.deepin.Session.xml
    impl/session.h
    Session)
qt5_add_dbus_adaptor(ADAPTER_SOURCES
    ../../dbus/adaptor/com.deepin.SessionManager.xml
    impl/session_manager.h
    SessionManager)

# dbus interface
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.deepin.daemon.PowerManager1.xml PowerManager org_deepin_PowerManager1)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.deepin.system.Bluetooth1.xml Bluetooth org_deepin_system_Bluetooth1)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.deepin.daemon.Daemon1.xml SessionDaemon org_deepin_daemon_Daemon1)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.login1.Manager.xml Login1Manager org_freedesktop_login1_Manager)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.login1.Session.xml Login1Session org_freedesktop_login1_Session)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.login1.User.xml Login1User org_freedesktop_login1_User)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.DBus.xml DBus org_freedesktop_DBus)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.systemd1.Manager.xml Systemd1Manager org_freedesktop_systemd1_Manager)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.systemd1.Job.xml Systemd1Job org_freedesktop_systemd1_Job)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.deepin.daemon.Audio1.xml Audio1 org_deepin_daemon_Audio1)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.deepin.daemon.Audio1.Sink.xml Sink org_deepin_daemon_Audio1_Sink)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES com.deepin.XSettings.xml XSettings com_deepin_XSettings)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES com.deepin.api.SoundThemePlayer.xml SoundThemePlayer com_deepin_api_SoundThemePlayer)

file(GLOB_RECURSE DBUS_TYPES ${PROJECT_SOURCE_DIR}/dbus/types/*)

add_executable(dde-session
    main.cpp
    environments_manager.h
    environments_manager.cpp
    impl/session.h
    impl/session.cpp
    impl/session_manager.h
    impl/session_manager.cpp
    ../utils/dconf.h
    ../utils/dconf.cpp
    ../utils/fifo.h
    ../utils/fifo.cpp
    ../utils/utils.h
    ${ADAPTER_SOURCES}
    ${INTERFACE_SOURCES}
    ${DBUS_TYPES}
    ${DBUSEXTENDED_SOURCE}
    )

target_link_libraries(dde-session
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Concurrent
    PkgConfig::Systemd
    ${DtkCore_LIBRARIES}
    PkgConfig::GLIB2
    PkgConfig::SECRET1
    PkgConfig::QGSettings
    )

target_include_directories(dde-session PUBLIC
    ../
    ${QT_DBUS_INTERFACE_INCLUDE}
    ${DBUSEXTENDED_INCLUDE}
    ${DBUS_INCLUDE}
    )

install(TARGETS dde-session DESTINATION bin)
