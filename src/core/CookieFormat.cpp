// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "CookieFormat.h"

#include <QByteArrayList>

namespace
{
QByteArray formatLine()
{
    return QByteArrayLiteral("crossparty-cookies 1");
}

bool isExpired(const QNetworkCookie &cookie, const QDateTime &now)
{
    return !cookie.isSessionCookie() && cookie.expirationDate() <= now;
}

QString hostWithoutLeadingDot(const QNetworkCookie &cookie)
{
    QString host = cookie.domain();
    if (host.startsWith(QLatin1Char('.'))) {
        host.remove(0, 1);
    }
    return host;
}
}

QByteArray CookieFormat::toBytes(const QList<QNetworkCookie> &cookies, const QDateTime &now)
{
    QByteArrayList lines{formatLine()};
    for (const QNetworkCookie &cookie : cookies) {
        if (!isExpired(cookie, now)) {
            // Base64 keeps each cookie on exactly one line, whatever it holds.
            lines.append(cookie.toRawForm(QNetworkCookie::Full).toBase64());
        }
    }
    return lines.join('\n');
}

QList<QNetworkCookie> CookieFormat::fromBytes(const QByteArray &bytes)
{
    const QByteArrayList lines = bytes.split('\n');
    if (lines.isEmpty() || lines.constFirst() != formatLine()) {
        return {};
    }

    QList<QNetworkCookie> cookies;
    for (qsizetype index = 1; index < lines.size(); ++index) {
        const auto decoded = QByteArray::fromBase64Encoding(lines.at(index), QByteArray::AbortOnBase64DecodingErrors);
        if (decoded) {
            cookies.append(QNetworkCookie::parseCookies(*decoded));
        }
    }
    return cookies;
}

QNetworkCookie CookieFormat::prepareForRestore(const QNetworkCookie &cookie)
{
    QNetworkCookie restored = cookie;
    if (!cookie.domain().startsWith(QLatin1Char('.'))) {
        restored.setDomain(QString());
    }
    return restored;
}

QUrl CookieFormat::originFor(const QNetworkCookie &cookie)
{
    QUrl origin;
    origin.setScheme(QStringLiteral("https"));
    origin.setHost(hostWithoutLeadingDot(cookie));
    return origin;
}
