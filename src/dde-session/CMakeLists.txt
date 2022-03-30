find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus REQUIRED)

qt5_add_dbus_adaptor(ADAPTER_SOURCES
                     ../../dbus/org.deepin.Session.xml
                     impl/session.h
                     Session)

add_executable(dde-session
  main.cpp
  impl/session.h
  impl/session.cpp
  ${ADAPTER_SOURCES}
  ../../dbus/org.deepin.Session.xml
)
target_link_libraries(dde-session
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
)

install(TARGETS dde-session DESTINATION bin)
