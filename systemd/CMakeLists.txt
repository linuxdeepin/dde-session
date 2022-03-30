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
    org.deepin.Session.service
)

install(FILES ${SERVICES}
        DESTINATION lib/systemd/user/)
