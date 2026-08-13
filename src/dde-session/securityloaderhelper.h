// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SECURITYLOADERHELPER_H
#define SECURITYLOADERHELPER_H

#include <QByteArray>
#include <QDBusConnection>
#include <QString>

/*!
 * @brief 安全加载器管道信息，用于与 deepin-security-loader 进程通信
 *
 * 当 dde-session 通过 deepin-security-loader 启动时，加载器会传入两个
 * 文件描述符（--fd1, --fd2），分别用于向加载器发送鉴权请求和接收响应。
 * 该结构体由 parseSecurityLoaderFds() 填充，生命周期由调用者管理。
 */
struct SecurityLoaderInfo
{
    /*! 是否通过安全加载器启动（即命令行包含 --fd1/--fd2 参数） */
    bool loaded = false;
    /*! 写入端管道 fd，用于向加载器发送鉴权请求，loaded 为 false 时无效 */
    int requestFd = -1;
    /*! 读取端管道 fd，用于接收加载器鉴权响应，loaded 为 false 时无效 */
    int responseFd = -1;
};
/*!
 * @brief 获取系统总线连接（单例）
 * @return 系统总线 QDBusConnection 引用，生命周期与进程一致
 */
const QDBusConnection &systemBusConnection();
/*!
 * @brief 解析命令行 --fd1/--fd2 参数为 SecurityLoaderInfo
 * @param hasRequestFd  是否包含 --fd1 参数
 * @param requestFd     --fd1 的参数值，文件描述符的数字字符串
 * @param hasResponseFd 是否包含 --fd2 参数
 * @param responseFd    --fd2 的参数值，文件描述符的数字字符串
 * @param errorMessage  可选的错误输出；传入 nullptr 时忽略错误文本
 * @return 解析后的 SecurityLoaderInfo；loaded 字段指示是否成功
 */
SecurityLoaderInfo parseSecurityLoaderFds(bool hasRequestFd,
                                          const QString &requestFd,
                                          bool hasResponseFd,
                                          const QString &responseFd,
                                          QString *errorMessage);
/*!
 * @brief 构建发送给安全加载器的 Power1 鉴权请求 JSON
 * @param uniqueName D-Bus 系统总线唯一名称（如 ":1.42"）
 * @return Compact JSON 字节数组，包含 UniqueName 和 DestList 字段
 */
QByteArray buildPowerAuthorizationRequest(const QString &uniqueName);
/*!
 * @brief 通过安全加载器鉴权 org.deepin.dde.Power1 接口的调用者
 * @param info        安全加载器管道信息
 * @param uniqueName  D-Bus 系统总线唯一名称
 * @param errorMessage 可选的错误输出；传入 nullptr 时忽略错误文本
 * @return true 鉴权通过（或未通过安全加载器启动）；false 鉴权失败
 * @note 当 info.loaded 为 false 时直接返回 true（非安全加载器启动路径）
 */
bool authorizePowerCaller(const SecurityLoaderInfo &info, const QString &uniqueName, QString *errorMessage);

#endif // SECURITYLOADERHELPER_H
