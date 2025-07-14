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
    dde-session-pre.target
    dde-session-core.target
    ${CMAKE_CURRENT_BINARY_DIR}/dde-session-exit-task.service
    ${CMAKE_CURRENT_BINARY_DIR}/dde-session-shutdown.service
    dde-session-shutdown.target
    dde-session.target
)
install(FILES ${SERVICES} DESTINATION lib/systemd/user/)

install(DIRECTORY DESTINATION lib/systemd/user/dde-session-pre.target.wants/)
set(DDE_SESSION_PRE_WANTS
    dde-session-pre.target.wants/dde-session@x11.service
    dde-session-pre.target.wants/dde-quick-login@x11.service
)

# dde-session-core.target.wants - core desktop components
install(DIRECTORY DESTINATION lib/systemd/user/dde-session-core.target.wants/)
set(DDE_SESSION_CORE_WANTS
    dde-session-core.target.wants/dde-shell@DDE.service
    dde-session-core.target.wants/dde-shell-plugin@org.deepin.ds.desktop.service
)

# dde-session-initialized.target.wants - other desktop services
install(DIRECTORY DESTINATION lib/systemd/user/dde-session-initialized.target.wants/)
set(DDE_SESSION_INITIALIZED_WANTS
    dde-session-initialized.target.wants/dde-lock.service
    dde-session-initialized.target.wants/dde-polkit-agent.service
)

install(FILES ${DDE_SESSION_PRE_WANTS} DESTINATION lib/systemd/user/)
install(FILES ${DDE_SESSION_CORE_WANTS} DESTINATION lib/systemd/user/)
install(FILES ${DDE_SESSION_INITIALIZED_WANTS} DESTINATION lib/systemd/user/)
install_symlink(dde-session@x11.service dde-session-pre.target.wants)
install_symlink(dde-quick-login@x11.service dde-session-pre.target.wants)
install_symlink(dde-shell@DDE.service dde-session-core.target.wants)
install_symlink(dde-shell-plugin@org.deepin.ds.desktop.service dde-session-core.target.wants)
install_symlink(dde-lock.service dde-session-initialized.target.wants)
install_symlink(dde-polkit-agent.service dde-session-initialized.target.wants)

install(FILES ${DDE_SESSION_DAEMON_WANTS} DESTINATION lib/systemd/user/)
install(DIRECTORY dde-session-shutdown.target.d DESTINATION lib/systemd/user/)
