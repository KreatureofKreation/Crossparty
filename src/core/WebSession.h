// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QQuickWebEngineProfile>
#include <QUrl>
#include <QWebEngineNavigationRequest>
#include <QWebEngineNewWindowRequest>
#include <QWebEngineNotification>
#include <QWebEnginePermission>
#include <QWebEngineScript>

#include "InviteWatch.h"

class CookieVault;
class SecretPortal;

// Connects the web view in the interface to the security policies in the
// core. The interface hands every navigation, new window and permission
// request to this object, and only this object decides.
class WebSession : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QQuickWebEngineProfile *profile READ profile CONSTANT)
    Q_PROPERTY(QUrl startUrl READ startUrl CONSTANT)
    // False until saved cookies have been restored (or there are none to
    // restore), so the page never loads before the user's sign-in is back.
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)
    // The read-only invite watcher the page view runs (InviteWatch).
    Q_PROPERTY(QWebEngineScript inviteWatchScript READ inviteWatchScript CONSTANT)

public:
    explicit WebSession(QObject *parent = nullptr);

    QQuickWebEngineProfile *profile() const;
    QUrl startUrl() const;
    bool isReady() const;
    QWebEngineScript inviteWatchScript() const;

    Q_INVOKABLE void handleNavigation(QWebEngineNavigationRequest *request) const;
    // The address goes to the browser, or is blocked. The party page is
    // never replaced.
    Q_INVOKABLE void handleNewWindow(QWebEngineNewWindowRequest *request) const;
    Q_INVOKABLE void handlePermission(const QWebEnginePermission &permission) const;
    // Signs out and erases all session data, then quits: the profile lives
    // only in memory, so closing the app is what erases page storage. See
    // docs/security.md#sec-018. If the saved sign-in cannot be deleted, it
    // stays open and emits wipeFailed instead.
    Q_INVOKABLE void signOutAndWipe();
    // Every console message from the page. Invite watcher reports are acted
    // on; anything else is only logged at debug level.
    Q_INVOKABLE void handleConsoleMessage(const QString &message, const QUrl &pageUrl);

Q_SIGNALS:
    void readyChanged();
    // A browser notification from an allowed page, such as a party invite.
    // The text is Microsoft's and must be shown as plain text only.
    void notificationArrived(const QString &title, const QString &message);
    // The page's invite count went up. Carries no names or other page text.
    void inviteArrived();
    // The saved sign-in file could not be deleted. The user must delete it
    // themselves, or it signs them back in next time.
    void wipeFailed(const QString &filePath);

private:
    void onNotification(QWebEngineNotification *notification);
    void onSecretReady(const QByteArray &secret);
    void onSecretUnavailable(const QString &reason);
    void setReady();

    QQuickWebEngineProfile *m_profile;
    CookieVault *m_cookieVault;
    SecretPortal *m_secretPortal;
    InviteWatch m_inviteWatch;
    bool m_ready = false;
};
