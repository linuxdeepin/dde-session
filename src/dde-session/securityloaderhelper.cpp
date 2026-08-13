// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "securityloaderhelper.h"

#include <QDBusConnectionInterface>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <cerrno>
#include <climits>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {
constexpr qsizetype MaxResponseSize = 1024 * 1024;
constexpr int HandshakeTimeoutMs = 5000;
void setErrorMessage(QString *errorMessage, const QString &message)
{
    if (errorMessage)
        *errorMessage = message;
}


bool parseFd(const QString &value, int *fd)
{
    bool ok = false;
    const qlonglong parsed = value.toLongLong(&ok, 10);
    if (!ok || parsed < 3 || parsed > INT_MAX)
        return false;
    *fd = static_cast<int>(parsed);
    return true;
}

bool validatePipeFd(int fd, int expectedAccessMode, QString *errorMessage)
{
    struct stat status;
    if (fstat(fd, &status) != 0) {
        setErrorMessage(errorMessage, QStringLiteral("cannot inspect loader pipe: %1")
                                           .arg(QString::fromLocal8Bit(strerror(errno))));
        return false;
    }
    if (!S_ISFIFO(status.st_mode)) {
        setErrorMessage(errorMessage, QStringLiteral("loader file descriptor is not a pipe"));
        return false;
    }

    const int flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        setErrorMessage(errorMessage, QStringLiteral("cannot inspect loader pipe access mode: %1")
                                           .arg(QString::fromLocal8Bit(strerror(errno))));
        return false;
    }
    if ((flags & O_ACCMODE) != expectedAccessMode) {
        setErrorMessage(errorMessage, QStringLiteral("loader pipe has an unexpected access mode"));
        return false;
    }
    return true;
}

bool closeFd(int fd, QString *errorMessage)
{
    if (close(fd) == 0 || errno == EINTR)
        return true;

    setErrorMessage(errorMessage, QStringLiteral("cannot close loader pipe: %1")
                                       .arg(QString::fromLocal8Bit(strerror(errno))));
    return false;
}

bool writeRequest(int fd, const QByteArray &request, QElapsedTimer *timer, QString *errorMessage)
{
    const int flags = fcntl(fd, F_GETFL);
    if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        setErrorMessage(errorMessage, QStringLiteral("cannot make loader request pipe non-blocking: %1")
                                           .arg(QString::fromLocal8Bit(strerror(errno))));
        closeFd(fd, nullptr);
        return false;
    }

    qsizetype written = 0;
    while (written < request.size()) {
        const int remaining = HandshakeTimeoutMs - static_cast<int>(timer->elapsed());
        if (remaining <= 0) {
            setErrorMessage(errorMessage, QStringLiteral("timed out writing security loader request"));
            closeFd(fd, nullptr);
            return false;
        }

        pollfd descriptor = {fd, POLLOUT, 0};
        int result;
        do {
            result = poll(&descriptor, 1, remaining);
        } while (result < 0 && errno == EINTR);
        if (result == 0) {
            setErrorMessage(errorMessage, QStringLiteral("timed out writing security loader request"));
            closeFd(fd, nullptr);
            return false;
        }
        if (result < 0) {
            setErrorMessage(errorMessage, QStringLiteral("cannot poll loader request: %1")
                                               .arg(QString::fromLocal8Bit(strerror(errno))));
            closeFd(fd, nullptr);
            return false;
        }

        const ssize_t count = write(fd, request.constData() + written,
                                    static_cast<size_t>(request.size() - written));
        if (count > 0) {
            written += count;
            continue;
        }
        if (count < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
            continue;
        setErrorMessage(errorMessage, QStringLiteral("cannot write loader request: %1")
                                           .arg(QString::fromLocal8Bit(strerror(errno))));
        closeFd(fd, nullptr);
        return false;
    }
    return closeFd(fd, errorMessage);
}

bool readResponse(int fd, QByteArray *response, QElapsedTimer *timer, QString *errorMessage)
{
    const int flags = fcntl(fd, F_GETFL);
    if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        setErrorMessage(errorMessage, QStringLiteral("cannot make loader response pipe non-blocking: %1")
                                           .arg(QString::fromLocal8Bit(strerror(errno))));
        closeFd(fd, nullptr);
        return false;
    }

    char buffer[4096];
    while (true) {
        const int remaining = HandshakeTimeoutMs - static_cast<int>(timer->elapsed());
        if (remaining <= 0) {
            setErrorMessage(errorMessage, QStringLiteral("timed out waiting for security loader response"));
            closeFd(fd, nullptr);
            return false;
        }

        pollfd descriptor = {fd, POLLIN | POLLHUP, 0};
        int result;
        do {
            result = poll(&descriptor, 1, remaining);
        } while (result < 0 && errno == EINTR);
        if (result == 0) {
            setErrorMessage(errorMessage, QStringLiteral("timed out waiting for security loader response"));
            closeFd(fd, nullptr);
            return false;
        }
        if (result < 0) {
            setErrorMessage(errorMessage, QStringLiteral("cannot poll loader response: %1")
                                               .arg(QString::fromLocal8Bit(strerror(errno))));
            closeFd(fd, nullptr);
            return false;
        }

        while (true) {
            const ssize_t count = read(fd, buffer, sizeof(buffer));
            if (count > 0) {
                if (response->size() + count > MaxResponseSize) {
                    setErrorMessage(errorMessage, QStringLiteral("security loader response exceeds size limit"));
                    closeFd(fd, nullptr);
                    return false;
                }
                response->append(buffer, count);
                continue;
            }
            if (count == 0)
                return closeFd(fd, errorMessage);
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            setErrorMessage(errorMessage, QStringLiteral("cannot read loader response: %1")
                                               .arg(QString::fromLocal8Bit(strerror(errno))));
            closeFd(fd, nullptr);
            return false;
        }
    }
}
}

const QDBusConnection &systemBusConnection()
{
    static const QDBusConnection connection = QDBusConnection::systemBus();
    return connection;
}

SecurityLoaderInfo parseSecurityLoaderFds(bool hasRequestFd,
                                          const QString &requestFd,
                                          bool hasResponseFd,
                                          const QString &responseFd,
                                          QString *errorMessage)
{
    SecurityLoaderInfo info;
    info.loaded = hasRequestFd || hasResponseFd;
    if (!info.loaded)
        return info;
    if (!hasRequestFd || !hasResponseFd) {
        setErrorMessage(errorMessage, QStringLiteral("security loader requires both --fd1 and --fd2"));
        return info;
    }
    if (!parseFd(requestFd, &info.requestFd) || !parseFd(responseFd, &info.responseFd)) {
        setErrorMessage(errorMessage, QStringLiteral("security loader file descriptors are invalid"));
        info.requestFd = -1;
        info.responseFd = -1;
        return info;
    }
    if (info.requestFd == info.responseFd) {
        setErrorMessage(errorMessage, QStringLiteral("security loader file descriptors must be distinct"));
        closeFd(info.requestFd, nullptr);
        info.requestFd = -1;
        info.responseFd = -1;
    }
    return info;
}

QByteArray buildPowerAuthorizationRequest(const QString &uniqueName)
{
    QJsonObject destination;
    destination.insert(QStringLiteral("DbusName"), QStringLiteral("org.deepin.dde.Power1"));
    destination.insert(QStringLiteral("DbusPath"), QStringLiteral("/org/deepin/dde/Power1"));
    destination.insert(QStringLiteral("DbusInterface"), QStringLiteral("org.deepin.dde.Power1"));

    QJsonObject request;
    request.insert(QStringLiteral("UniqueName"), uniqueName);
    request.insert(QStringLiteral("DestList"), QJsonArray{destination});
    return QJsonDocument(request).toJson(QJsonDocument::Compact);
}

bool authorizePowerCaller(const SecurityLoaderInfo &info, const QString &uniqueName, QString *errorMessage)
{
    if (!info.loaded)
        return true;
    if (info.requestFd < 0 || info.responseFd < 0) {
        setErrorMessage(errorMessage, QStringLiteral("invalid file descriptors"));
        if (info.requestFd >= 0)
            closeFd(info.requestFd, nullptr);
        if (info.responseFd >= 0)
            closeFd(info.responseFd, nullptr);
        return false;
    }
    if (!validatePipeFd(info.requestFd, O_WRONLY, errorMessage)
            || !validatePipeFd(info.responseFd, O_RDONLY, errorMessage)) {
        closeFd(info.requestFd, nullptr);
        closeFd(info.responseFd, nullptr);
        return false;
    }

    if (uniqueName.isEmpty() || !uniqueName.startsWith(QLatin1Char(':'))) {
        setErrorMessage(errorMessage, QStringLiteral("system bus unique name is unavailable"));
        closeFd(info.requestFd, nullptr);
        closeFd(info.responseFd, nullptr);
        return false;
    }

    QElapsedTimer timer;
    timer.start();
    if (!writeRequest(info.requestFd, buildPowerAuthorizationRequest(uniqueName), &timer, errorMessage)) {
        closeFd(info.responseFd, nullptr);
        return false;
    }

    QByteArray response;
    if (!readResponse(info.responseFd, &response, &timer, errorMessage))
        return false;

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(response, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        setErrorMessage(errorMessage, QStringLiteral("invalid security loader response"));
        return false;
    }
    const QJsonObject result = document.object();
    if (!result.value(QStringLiteral("Result")).toBool(false)) {
        QString msg = result.value(QStringLiteral("Message")).toString(QStringLiteral("authorization denied"));
        for (QChar &character : msg) {
            if (!character.isPrint())
                character = QLatin1Char(' ');
        }
        setErrorMessage(errorMessage, msg);
        return false;
    }
    return true;
}
