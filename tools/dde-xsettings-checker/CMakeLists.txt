set(BIN_NAME dde-xsettings-checker)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSettings REQUIRED IMPORTED_TARGET gsettings-qt)

message(${PROJECT_SOURCE_DIR})

add_executable(${BIN_NAME}
    main.cpp
    xsettingschecker.h
    xsettingschecker.cpp
    ${PROJECT_SOURCE_DIR}/src/utils/utils.h
    )

target_link_libraries(${BIN_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    PkgConfig::QGSettings
    )

target_include_directories(${BIN_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/src/utils
    )

install(TARGETS ${BIN_NAME} DESTINATION bin)

set(SYSTEMD_FILES
    dde-xsettings-checker.service
)
install(FILES ${SYSTEMD_FILES} DESTINATION lib/systemd/user/)
install(DIRECTORY DESTINATION lib/systemd/user/dde-session-daemon.target.wants/)
install_symlink(dde-xsettings-checker.service dde-session-daemon.target.wants)
