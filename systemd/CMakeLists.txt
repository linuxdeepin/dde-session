configure_file(dde-session-manager.service.in
               dde-session-manager.service
               @ONLY)
configure_file(dde-session-exit-task.service.in
               dde-session-exit-task.service
               @ONLY)
configure_file(dde-session-shutdown.service.in
               dde-session-shutdown.service
               @ONLY)

set(SERVICES
    dde-session-initialized.target
    ${CMAKE_CURRENT_BINARY_DIR}/dde-session-manager.service
    dde-session-manager.target
    dde-session-pre.target
    ${CMAKE_CURRENT_BINARY_DIR}/dde-session-exit-task.service
    ${CMAKE_CURRENT_BINARY_DIR}/dde-session-shutdown.service
    dde-session-shutdown.target
    dde-session.target
    dde-session@x11.service
    dde-session@wayland.service
    services.slice
)
install(FILES ${SERVICES} DESTINATION lib/systemd/user/)

# dde-session-initialized.target.wants - TODO: 暂时先放这里，后续应该转移到对应服务
install(DIRECTORY DESTINATION lib/systemd/user/dde-session-initialized.target.wants/)
set(DDE_SESSION_INITIALIZED_WANTS
    dde-session-initialized.target.wants/dde-display.service
    dde-session-initialized.target.wants/dde-display.target
    dde-session-initialized.target.wants/dde-desktop.service
    dde-session-initialized.target.wants/dde-session-daemon.service
    dde-session-initialized.target.wants/dde-session-daemon.target
    dde-session-initialized.target.wants/dde-dock.service
)
install(FILES ${DDE_SESSION_INITIALIZED_WANTS} DESTINATION lib/systemd/user/)
install_symlink(dde-display.service dde-session-initialized.target.wants)
install_symlink(dde-desktop.service dde-session-initialized.target.wants)
install_symlink(dde-session-daemon.service dde-session-initialized.target.wants)

# dde-session-daemon.target.wants - TODO: 暂时先放这里，后续应该转移到对应服务
install(DIRECTORY DESTINATION lib/systemd/user/dde-session-daemon.target.wants/)
set(DDE_SESSION_DAEMON_WANTS
    dde-session-daemon.target.wants/dde-lock.service
    dde-session-daemon.target.wants/dde-lock.timer
    dde-session-daemon.target.wants/dde-osd.service
    dde-session-daemon.target.wants/dde-osd.timer
    dde-session-daemon.target.wants/dde-osd.target
    dde-session-daemon.target.wants/dde-polkit-agent.service
    dde-session-daemon.target.wants/dde-polkit-agent.timer
)
install(FILES ${DDE_SESSION_DAEMON_WANTS} DESTINATION lib/systemd/user/)
install_symlink(dde-dock.service dde-session-daemon.target.wants)
install_symlink(dde-lock.timer dde-session-daemon.target.wants)
install_symlink(dde-osd.timer dde-session-daemon.target.wants)
install_symlink(dde-polkit-agent.timer dde-session-daemon.target.wants)
