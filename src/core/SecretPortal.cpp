// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "SecretPortal.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusUnixFileDescriptor>

#include <cerrno>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>

namespace
{
const char portalService[] = "org.freedesktop.portal.Desktop";
const char portalPath[] = "/org/freedesktop/portal/desktop";
const char requestToken[] = "crossparty_secret";

// KWallet may first ask the user to unlock it. After this long the app goes
// on without saved cookies rather than wait on a blank window.
constexpr std::chrono::seconds unlockTimeout{60};

// The portal answers on a Request object whose path is built from our bus
// name and the token we choose. We listen there before calling, so the answer
// cannot arrive before we are ready for it.
QString requestPath()
{
    QString sender = QDBusConnection::sessionBus().baseService().mid(1);
    sender.replace(QLatin1Char('.'), QLatin1Char('_'));
    return QStringLiteral("/org/freedesktop/portal/desktop/request/%1/%2").arg(sender, QLatin1String(requestToken));
}
}

SecretPortal::SecretPortal(QObject *parent)
    : QObject(parent)
{
    m_timeout.setSingleShot(true);
    m_timeout.setInterval(unlockTimeout);
    connect(&m_timeout, &QTimer::timeout, this, [this] {
        fail(QStringLiteral("no answer from the Secret portal"));
    });
}

SecretPortal::~SecretPortal()
{
    closeReadEnd();
}

void SecretPortal::requestSecret()
{
    // The portal writes the secret into a pipe we hand it, so the secret never
    // travels over D-Bus where other programs on the bus could see it.
    int pipeEnds[2] = {-1, -1};
    if (pipe2(pipeEnds, O_CLOEXEC | O_NONBLOCK) != 0) {
        fail(QStringLiteral("could not create a pipe"));
        return;
    }
    m_readEnd = pipeEnds[0];
    // QDBusUnixFileDescriptor keeps its own copy of the write end, so ours is
    // closed straight away. The portal closes its copy when it has written.
    const QDBusUnixFileDescriptor writeEnd(pipeEnds[1]);
    close(pipeEnds[1]);

    QDBusConnection bus = QDBusConnection::sessionBus();
    // Qt's D-Bus signals only connect by name. The formatter is kept off this
    // line because it would add a space that clazy rejects.
    // clang-format off
    bus.connect(QLatin1String(portalService), requestPath(), QStringLiteral("org.freedesktop.portal.Request"), QStringLiteral("Response"),
                this, SLOT(onResponse(uint,QVariantMap)));
    // clang-format on

    QDBusMessage call = QDBusMessage::createMethodCall(QLatin1String(portalService),
                                                       QLatin1String(portalPath),
                                                       QStringLiteral("org.freedesktop.portal.Secret"),
                                                       QStringLiteral("RetrieveSecret"));
    call.setArguments({QVariant::fromValue(writeEnd), QVariantMap{{QStringLiteral("handle_token"), QLatin1String(requestToken)}}});
    const QDBusMessage reply = bus.call(call);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        fail(reply.errorMessage());
        return;
    }
    m_timeout.start();
}

void SecretPortal::onResponse(uint response, const QVariantMap &results)
{
    Q_UNUSED(results)
    m_timeout.stop();
    if (response != 0) {
        fail(QStringLiteral("the Secret portal refused (response %1)").arg(response));
        return;
    }

    const QByteArray secret = readSecret();
    closeReadEnd();
    if (secret.isEmpty()) {
        fail(QStringLiteral("the Secret portal sent an empty secret"));
        return;
    }
    Q_EMIT secretReady(secret);
}

QByteArray SecretPortal::readSecret() const
{
    QByteArray secret;
    char buffer[256];
    while (true) {
        const ssize_t count = read(m_readEnd, buffer, sizeof buffer);
        if (count > 0) {
            secret.append(buffer, count);
        } else if (count < 0 && errno == EINTR) {
            continue;
        } else {
            // End of the pipe, or nothing more waiting in it.
            return secret;
        }
    }
}

void SecretPortal::fail(const QString &reason)
{
    m_timeout.stop();
    closeReadEnd();
    Q_EMIT secretUnavailable(reason);
}

void SecretPortal::closeReadEnd()
{
    if (m_readEnd >= 0) {
        close(m_readEnd);
        m_readEnd = -1;
    }
}
