find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus REQUIRED)

qt5_add_dbus_interface(DBUS_SOURCES
    ../../dbus/org.deepin.Session.xml
    Session
)

add_executable(dde-session-ctl
  main.cpp
  ${DBUS_SOURCES}
  ../../dbus/org.deepin.Session.xml
)
target_link_libraries(dde-session-ctl
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
)

install(TARGETS dde-session-ctl DESTINATION lib/libexec/)
