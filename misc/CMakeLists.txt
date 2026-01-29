configure_file(xsessions/deepin.desktop.in xsessions/deepin.desktop)

set(XSESSION
    ${CMAKE_CURRENT_SOURCE_DIR}/Xsession.d/00deepin-dde-env
    ${CMAKE_CURRENT_SOURCE_DIR}/Xsession.d/01deepin-profile
    ${CMAKE_CURRENT_SOURCE_DIR}/Xsession.d/97deepin-keyring-wb
)

set(PROFILE
    ${CMAKE_CURRENT_SOURCE_DIR}/profile.d/deepin-xdg-dir.sh
)

install(
    DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/xsessions
    DESTINATION
        ${CMAKE_INSTALL_DATADIR}
    PATTERN
        "*.in" EXCLUDE
)

install(FILES ${XSESSION} DESTINATION /etc/X11/Xsession.d/)
install(FILES ${PROFILE} DESTINATION /etc/profile.d)

file(GLOB DCONFIG_FILES "dconf/*.json")
dtk_add_config_meta_files(APPID org.deepin.dde.session FILES ${DCONFIG_FILES})
