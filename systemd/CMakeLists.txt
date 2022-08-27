
macro(install_symlink filepath wantsdir)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/link/${wantsdir}/)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/lib/systemd/user/${filepath} ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath})
    install(FILES ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath} DESTINATION lib/systemd/user/${wantsdir}/)
endmacro(install_symlink)

set(SERVICES
    dde-session-initialized.target
    dde-session-manager.service
    dde-session-manager.target
    dde-session-pre.target
    dde-session-exit-task.service
    dde-session-shutdown.service
    dde-session-shutdown.target
    dde-session.target
    dde-session-x11.target
    dde-session-wayland.target
    services.slice
)
install(FILES ${SERVICES} DESTINATION lib/systemd/user/)

# dde-session-initialized.target.wants - 暂时先放这里，后续应该转移到对应服务
install(DIRECTORY DESTINATION lib/systemd/user/dde-session-initialized.target.wants/)
set(DDE_SESSION_INITIALIZED_WANTS
    dde-session-initialized.target.wants/dde-display.service
    dde-session-initialized.target.wants/dde-display.target
    dde-session-initialized.target.wants/dde-desktop.service
    dde-session-initialized.target.wants/dde-session-daemon.service
    dde-session-initialized.target.wants/dde-session-daemon.target
)
install(FILES ${DDE_SESSION_INITIALIZED_WANTS} DESTINATION lib/systemd/user/)
install_symlink(dde-display.service dde-session-initialized.target.wants)
install_symlink(dde-desktop.service dde-session-initialized.target.wants)
install_symlink(dde-session-daemon.service dde-session-initialized.target.wants)


# dde-session-daemon.target.wants - 暂时先放这里，后续应该转移到对应服务
install(DIRECTORY DESTINATION lib/systemd/user/dde-session-daemon.target.wants/)
set(DDE_SESSION_DAEMON_WANTS
    dde-session-daemon.target.wants/dde-dock.service
    dde-session-daemon.target.wants/dde-lock.service
    dde-session-daemon.target.wants/dde-lock.timer
    dde-session-daemon.target.wants/dde-osd.service
    dde-session-daemon.target.wants/dde-osd.timer
    dde-session-daemon.target.wants/dde-polkit-agent.service
    dde-session-daemon.target.wants/dde-polkit-agent.timer
)
install(FILES ${DDE_SESSION_DAEMON_WANTS} DESTINATION lib/systemd/user/)
install_symlink(dde-dock.service dde-session-daemon.target.wants)
install_symlink(dde-lock.timer dde-session-daemon.target.wants)
install_symlink(dde-osd.timer dde-session-daemon.target.wants)
install_symlink(dde-polkit-agent.timer dde-session-daemon.target.wants)