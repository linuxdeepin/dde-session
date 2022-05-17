
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
    dde-session-restart-dbus.service
    dde-session-shutdown.service
    dde-session-shutdown.target
    dde-session.target
    dde-session-x11-services-ready.target
    dde-session-x11-services.target
    dde-session-x11.target
    dde-session-wayland.target
    services.slice
)

install(FILES ${SERVICES} DESTINATION lib/systemd/user/)

# dde-session-pre.target.wants - WantsBy=dde-session-pre.target的项目，service暂时先放这里，后续应该转移到对应服务
install(DIRECTORY DESTINATION lib/systemd/user/dde-session-pre.target.wants/)
set(DDE_SESSION_PRE_WANTS
    dde-session-pre.target.wants/dde-display.service
    dde-session-pre.target.wants/dde-display.target
)
install(FILES ${DDE_SESSION_PRE_WANTS} DESTINATION lib/systemd/user/)

# install_symlink(dde-display.target dde-session-pre.target.wants)

# dde-session-initialized.target.wants - WantsBy=dde-session-initialized.target的项目，service暂时先放这里，后续应该转移到对应服务
install(DIRECTORY DESTINATION lib/systemd/user/dde-session-initialized.target.wants/)
set(DDE_SESSION_INITIALIZED_WANTS
    dde-session-initialized.target.wants/dde-desktop.service
    # dde-session-initialized.target.wants/dde-desktop.target
    dde-session-initialized.target.wants/dde-dock.service
    # dde-session-initialized.target.wants/dde-dock.target
    dde-session-initialized.target.wants/dde-lock.service
    # dde-session-initialized.target.wants/dde-lock.target
    dde-session-initialized.target.wants/dde-osd.service
    # dde-session-initialized.target.wants/dde-osd.target
    dde-session-initialized.target.wants/dde-polkit-agent.service
    # dde-session-initialized.target.wants/dde-polkit-agent.target
    dde-session-initialized.target.wants/dde-session-daemon.service
    dde-session-initialized.target.wants/dde-session-daemon.target
    dde-session-initialized.target.wants/deepin-deepinid-daemon.service
    # dde-session-initialized.target.wants/deepin-deepinid-daemon.target
)
install(FILES ${DDE_SESSION_INITIALIZED_WANTS} DESTINATION lib/systemd/user/)

# install_symlink(dde-desktop.target dde-session-initialized.target.wants)
# install_symlink(dde-session-daemon.target dde-session-initialized.target.wants)
# install_symlink(dde-dock.target dde-session-initialized.target.wants)
#install_symlink(dde-lock.target dde-session-initialized.target.wants)
#install_symlink(dde-osd.target dde-session-initialized.target.wants)
#install_symlink(dde-polkit-agent.target dde-session-initialized.target.wants)
#install_symlink(deepin-deepinid-daemon.target dde-session-initialized.target.wants)