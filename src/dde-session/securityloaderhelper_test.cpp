// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "securityloaderhelper.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <iostream>
#include <fcntl.h>
#include <unistd.h>

namespace {
bool require(bool condition, const char *message)
{
    if (condition)
        return true;
    std::cerr << message << std::endl;
    return false;
}
}

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    QString error;

    const SecurityLoaderInfo direct = parseSecurityLoaderFds(false, {}, false, {}, &error);
    if (!require(!direct.loaded && error.isEmpty(), "direct invocation was treated as loader invocation"))
        return 1;

    error.clear();
    const SecurityLoaderInfo incomplete = parseSecurityLoaderFds(true, QStringLiteral("7"), false, {}, &error);
    if (!require(incomplete.loaded && incomplete.requestFd < 0 && !error.isEmpty(),
                 "incomplete loader arguments were accepted"))
        return 1;

    const SecurityLoaderInfo incompleteWithoutError =
            parseSecurityLoaderFds(true, QStringLiteral("7"), false, {}, nullptr);
    if (!require(incompleteWithoutError.loaded && incompleteWithoutError.requestFd < 0,
                 "incomplete loader arguments crashed without an error output"))
        return 1;

    SecurityLoaderInfo invalidHandshake;
    invalidHandshake.loaded = true;
    if (!require(!authorizePowerCaller(invalidHandshake, QStringLiteral(":1.1"), nullptr),
                 "invalid descriptors were accepted without an error output"))
        return 1;

    error.clear();
    const SecurityLoaderInfo valid = parseSecurityLoaderFds(true, QStringLiteral("7"),
                                                            true, QStringLiteral("8"), &error);
    if (!require(valid.loaded && valid.requestFd == 7 && valid.responseFd == 8 && error.isEmpty(),
                 "valid loader arguments were rejected"))
        return 1;

    const QString uniqueName = QStringLiteral(":1.42");
    const QJsonDocument document = QJsonDocument::fromJson(buildPowerAuthorizationRequest(uniqueName));
    if (!require(document.isObject(), "authorization request is not a JSON object"))
        return 1;
    const QJsonObject request = document.object();
    if (!require(request.value(QStringLiteral("UniqueName")).toString() == uniqueName,
                 "authorization request has the wrong unique name"))
        return 1;
    const QJsonArray destinations = request.value(QStringLiteral("DestList")).toArray();
    if (!require(destinations.size() == 1, "authorization request has an unexpected destination count"))
        return 1;
    const QJsonObject power = destinations.at(0).toObject();
    if (!require(power.value(QStringLiteral("DbusName")).toString() == QStringLiteral("org.deepin.dde.Power1")
                 && power.value(QStringLiteral("DbusPath")).toString() == QStringLiteral("/org/deepin/dde/Power1")
                 && power.value(QStringLiteral("DbusInterface")).toString() == QStringLiteral("org.deepin.dde.Power1"),
                 "authorization request has the wrong Power1 destination"))
        return 1;

    int requestPipe[2];
    int responsePipe[2];
    if (!require(pipe2(requestPipe, O_CLOEXEC) == 0 && pipe2(responsePipe, O_CLOEXEC) == 0,
                 "cannot create handshake test pipes"))
        return 1;

    const QByteArray acceptedResponse = QByteArrayLiteral("{\"Result\":true,\"Message\":\"\"}");
    if (!require(write(responsePipe[1], acceptedResponse.constData(), acceptedResponse.size()) == acceptedResponse.size(),
                 "cannot seed loader response"))
        return 1;
    close(responsePipe[1]);

    SecurityLoaderInfo handshake;
    handshake.loaded = true;
    handshake.requestFd = requestPipe[1];
    handshake.responseFd = responsePipe[0];
    error.clear();
    const QString handshakeUniqueName = QStringLiteral(":1.99");
    if (!require(authorizePowerCaller(handshake, handshakeUniqueName, &error), "valid pipe handshake was rejected"))
        return 1;

    const QByteArray sentRequest = [&requestPipe] {
        QByteArray data;
        char buffer[1024];
        ssize_t count;
        while ((count = read(requestPipe[0], buffer, sizeof(buffer))) > 0)
            data.append(buffer, count);
        close(requestPipe[0]);
        return data;
    }();
    const QJsonObject sentObject = QJsonDocument::fromJson(sentRequest).object();
    const QString sentUniqueName = sentObject.value(QStringLiteral("UniqueName")).toString();
    if (!require(sentUniqueName == handshakeUniqueName,
                 "handshake did not send the supplied system bus unique name"))
        return 1;

    int deniedRequestPipe[2];
    int deniedResponsePipe[2];
    if (!require(pipe2(deniedRequestPipe, O_CLOEXEC) == 0
                         && pipe2(deniedResponsePipe, O_CLOEXEC) == 0,
                 "cannot create denied handshake test pipes"))
        return 1;

    const QByteArray deniedResponse = QByteArrayLiteral(
            "{\"Result\":false,\"Message\":\"denied\\n\\tesc\\u001b[31m\\u0007\"}");
    if (!require(write(deniedResponsePipe[1], deniedResponse.constData(), deniedResponse.size())
                         == deniedResponse.size(),
                 "cannot seed denied loader response"))
        return 1;
    close(deniedResponsePipe[1]);

    SecurityLoaderInfo deniedHandshake;
    deniedHandshake.loaded = true;
    deniedHandshake.requestFd = deniedRequestPipe[1];
    deniedHandshake.responseFd = deniedResponsePipe[0];
    error.clear();
    if (!require(!authorizePowerCaller(deniedHandshake, handshakeUniqueName, &error),
                 "denied pipe handshake was accepted"))
        return 1;
    close(deniedRequestPipe[0]);

    bool errorIsPrintable = true;
    for (const QChar character : error) {
        if (!character.isPrint()) {
            errorIsPrintable = false;
            break;
        }
    }
    if (!require(errorIsPrintable && error.contains(QStringLiteral("denied")),
                 "loader denial message retained control characters"))
        return 1;

    return 0;
}
