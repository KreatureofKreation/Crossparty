// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QByteArray>
#include <QHash>
#include <QNetworkCookie>
#include <QObject>
#include <QTimer>

class QWebEngineCookieStore;

// Keeps the web profile's cookies in an encrypted file, so sign-in survives a
// restart while nothing about it is stored as plain text. Until it has a key
// it saves nothing at all. See docs/security.md#sec-009.
class CookieVault : public QObject
{
    Q_OBJECT

public:
    CookieVault(QWebEngineCookieStore *cookieStore, const QString &filePath, QObject *parent = nullptr);

    // Derives the key from the portal secret and restores the saved cookies.
    // Returns how many cookies were restored, or -1 if there is no usable key.
    int unlockAndRestore(const QByteArray &portalSecret);

    // Writes the file now instead of waiting for the next quiet moment.
    void saveNow();

    // Sign out and wipe: forgets the key and every cookie it knew, stops
    // saving, and deletes the file. Returns false if the file could not be
    // deleted. See docs/security.md#sec-018.
    bool wipe();

private:
    void onCookieAdded(const QNetworkCookie &cookie);
    void onCookieRemoved(const QNetworkCookie &cookie);
    void scheduleSave();
    int restore();

    static QByteArray keyFor(const QNetworkCookie &cookie);

    QWebEngineCookieStore *m_cookieStore;
    QString m_filePath;
    QByteArray m_key;
    QHash<QByteArray, QNetworkCookie> m_cookies;
    QTimer m_saveTimer;
};
