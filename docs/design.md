# 会话与 systemd 生命周期

LightDM 认证通过后启动 `dde-session`。入口进程导入会话环境，启动
`dde-session.target`，并监听 `org.deepin.dde.Session1` 的注销。
`dde-session-manager.service` 通过 loader wrapper 运行
`dde-session --systemd-service`，注册 Session1 等 D-Bus 接口。
会话按 pre、core、initialized 阶段启动，依赖关系以 `systemd/` 中的单元为准。

## 桌面、Dock 和托盘

core 阶段并行启动桌面 `dde-shell-plugin@org.deepin.ds.desktop.service` 和
任务栏 `dde-shell@DDE.service`；两者之间没有启动或停止顺序约束。
任务栏属于 dde-shell 的 DDE 分组，不再由旧的 `dde-dock.service` 管理。

Dock 通过 `Wants=dde-tray-loader.target` 启动托盘。配套 dde-tray-loader 中，
target 通过 `After=` 等待 Dock 的 `Type=dbus` 启动完成：Dock 的内部合成器
就绪后才注册 `org.deepin.dde.Dock1`。随后各托盘分组服务并行启动。
target 通过 `BindsTo=` 和 `PartOf=` 绑定 Dock，无需直接绑定 core 或额外的 ready 服务。

停止时依赖顺序反转，各托盘组并行退出后 Dock 才退出。桌面可以同时退出，
不会等待 Dock；慢托盘仍会延迟 Dock 停止。这些顺序不保证合成器窗口动画或
屏幕残影的消失顺序。托盘生命周期测试见 dde-tray-loader 的 README。

## 注销与异常退出

正常注销经过 SessionManager 的注销准备流程。`dde-session-ctl --logout`
则调用 Session1 的 `Logout()`。管理进程退出后，systemd 执行
`dde-session-manager.service` 的 `ExecStopPost=dde-session-ctl --shutdown`，
启动 `dde-session-shutdown.target`，利用冲突关系停止会话目标与服务。
也可以通过 `dde-session-shutdown.service` 请求同一关闭流程。

管理进程被终止或崩溃时，`ExecStopPost` 仍执行清理，`OnFailure` 还会触发
shutdown target。因此无需在 POSIX 信号处理函数中调用 Qt 或 D-Bus；
此类重入可能等待被中断线程持有的锁，反而阻塞进程退出。

入口进程发现 Session1 总线名称消失后，启动 `dde-session-exit-task.service`
并退出，将控制权交回 LightDM。exit task 通过 `dde-session-ctl --session-exit`
在一秒延迟后停止用户 `dbus.service`。登录界面的后续启动与壁纸加载属于
LightDM/greeter 流程，应与会话服务的停止耗时分别排查。
