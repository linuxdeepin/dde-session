set(BIN_NAME dde-login-reminder)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network DBus REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Core REQUIRED)

find_package(PkgConfig REQUIRED)

message(${PROJECT_SOURCE_DIR})

add_executable(${BIN_NAME}
    main.cpp
    loginreminderinfo.h
    loginreminderinfo.cpp
    ${PROJECT_SOURCE_DIR}/src/utils/utils.h
)

target_link_libraries(${BIN_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Network
    Dtk${DTK_VERSION_MAJOR}::Core
)

target_include_directories(${BIN_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/src/utils
)

target_compile_definitions(${BIN_NAME} PRIVATE
    CMAKE_INSTALL_FULL_DATADIR="${CMAKE_INSTALL_FULL_DATADIR}"
)

install(TARGETS ${BIN_NAME} DESTINATION bin)

# Install desktop file to autostart directory
install(FILES dde-login-reminder.desktop DESTINATION /etc/xdg/autostart/)

# Translation files
file(GLOB TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS LinguistTools)
if(Qt${QT_VERSION_MAJOR}LinguistTools_FOUND AND TS_FILES)
    qt_add_translation(QM_FILES ${TS_FILES})
    add_custom_target(${BIN_NAME}_translations ALL DEPENDS ${QM_FILES})
    install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/${BIN_NAME}/translations)
endif()
