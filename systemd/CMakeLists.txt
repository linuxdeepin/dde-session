configure_file(dde-session-manager.service.in
               dde-session-manager.service
               @ONLY)
configure_file(dde-session-exit-task.service.in
               dde-session-exit-task.service
               @ONLY)
configure_file(dde-session-shutdown.service.in
               dde-session-shutdown.service
               @ONLY)
configure_file(dde-keyring.service.in
               dde-keyring.service
               @ONLY)
configure_file(dde-session-environment.service.in
               dde-session-environment.service
               @ONLY)
configure_file(dde-session-environment.sh.in
               dde-session-environment.sh
               @ONLY)


set(SERVICES
    dde-session-initialized.target
    ${CMAKE_CURRENT_BINARY_DIR}/dde-session-manager.service
    dde-session-pre.target
    dde-session-core.target
    ${CMAKE_CURRENT_BINARY_DIR}/dde-session-exit-task.service
    ${CMAKE_CURRENT_BINARY_DIR}/dde-session-shutdown.service
    ${CMAKE_CURRENT_BINARY_DIR}/dde-keyring.service
    ${CMAKE_CURRENT_BINARY_DIR}/dde-session-environment.service
    dde-session-shutdown.target
    dde-session.target
)
install(FILES ${SERVICES} DESTINATION lib/systemd/user/)

# Install the early environment setup script
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/dde-session-environment.sh
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/deepin)

# graphical-session-pre.target.wants - early environment injection
install(DIRECTORY DESTINATION lib/systemd/user/graphical-session-pre.target.wants/)
install_symlink(dde-session-environment.service graphical-session-pre.target.wants)

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
