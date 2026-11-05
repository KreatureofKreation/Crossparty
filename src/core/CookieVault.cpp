// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "CookieVault.h"

#include "CookieCipher.h"
#include "CookieFormat.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QSaveFile>
#include <QWebEngineCookieStore>

#include <chrono>

Q_LOGGING_CATEGORY(lcVault, "crossparty.cookies", QtInfoMsg)

namespace
{
// Sign-in sets many cookies in a burst; one save after it settles is enough.
constexpr std::chrono::seconds saveDelay{2};
}

CookieVault::CookieVault(QWebEngineCookieStore *cookieStore, const QString &filePath, QObject *parent)
    : QObject(parent)
    , m_cookieStore(cookieStore)
    , m_filePath(filePath)
{
    m_saveTimer.setSingleShot(true);
    m_saveTimer.setInterval(saveDelay);
    connect(&m_saveTimer, &QTimer::timeout, this, &CookieVault::saveNow);

    connect(m_cookieStore, &QWebEngineCookieStore::cookieAdded, this, &CookieVault::onCookieAdded);
    connect(m_cookieStore, &QWebEngineCookieStore::cookieRemoved, this, &CookieVault::onCookieRemoved);
}

int CookieVault::unlockAndRestore(const QByteArray &portalSecret)
{
    m_key = CookieCipher::deriveKey(portalSecret);
    if (m_key.isEmpty()) {
        qCWarning(lcVault) << "Could not make a key; sign-in will not be saved";
        return -1;
    }
    return restore();
}

int CookieVault::restore()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCInfo(lcVault) << "No saved cookies yet";
        return 0;
    }

    const std::optional<QByteArray> plaintext = CookieCipher::decrypt(file.readAll(), m_key);
    file.close();
    if (!plaintext) {
        // The file was changed, or was made with another key (for example the
        // wallet was reset). It cannot be trusted or read, so start signed out.
        qCWarning(lcVault) << "Saved cookies could not be decrypted; removing them";
        QFile::remove(m_filePath);
        return 0;
    }

    const QList<QNetworkCookie> cookies = CookieFormat::fromBytes(*plaintext);
    for (const QNetworkCookie &cookie : cookies) {
        m_cookieStore->setCookie(CookieFormat::prepareForRestore(cookie), CookieFormat::originFor(cookie));
    }
    qCInfo(lcVault) << "Restored" << cookies.size() << "cookies";
    return static_cast<int>(cookies.size());
}

void CookieVault::saveNow()
{
    m_saveTimer.stop();
    if (m_key.isEmpty()) {
        // Fail safe: with no key, nothing is written. Never plain text.
        return;
    }

    const QByteArray plaintext = CookieFormat::toBytes(m_cookies.values(), QDateTime::currentDateTimeUtc());
    const std::optional<QByteArray> encrypted = CookieCipher::encrypt(plaintext, m_key);
    if (!encrypted) {
        qCWarning(lcVault) << "Encryption failed; cookies not saved";
        return;
    }

    QDir().mkpath(QFileInfo(m_filePath).absolutePath());
    // QSaveFile writes to a temporary file and swaps it in, so a crash mid-save
    // never leaves a half-written file.
    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qCWarning(lcVault) << "Could not open the cookie file for writing";
        return;
    }
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    file.write(*encrypted);
    if (!file.commit()) {
        qCWarning(lcVault) << "Could not save the cookie file";
    }
}

void CookieVault::onCookieAdded(const QNetworkCookie &cookie)
{
    m_cookies.insert(keyFor(cookie), cookie);
    scheduleSave();
}

void CookieVault::onCookieRemoved(const QNetworkCookie &cookie)
{
    m_cookies.remove(keyFor(cookie));
    scheduleSave();
}

void CookieVault::scheduleSave()
{
    if (!m_key.isEmpty()) {
        m_saveTimer.start();
    }
}

// A cookie is identified by its domain, path and name; a new value for the
// same three replaces the old one.
QByteArray CookieVault::keyFor(const QNetworkCookie &cookie)
{
    return cookie.domain().toUtf8() + '\n' + cookie.path().toUtf8() + '\n' + cookie.name();
}
