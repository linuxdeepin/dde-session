configure_file(xsessions/dde-x11.desktop.in xsessions/dde-x11.desktop)
configure_file(wayland-sessions/dde-wayland.desktop.in wayland-sessions/dde-wayland.desktop)

install(
    DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/xsessions
        ${CMAKE_CURRENT_BINARY_DIR}/wayland-sessions
    DESTINATION
        ${CMAKE_INSTALL_DATADIR}
)
