// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QQuickWebEngineProfile>
#include <QUrl>
#include <QWebEngineNavigationRequest>
#include <QWebEngineNewWindowRequest>
#include <QWebEnginePermission>

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

public:
    explicit WebSession(QObject *parent = nullptr);

    QQuickWebEngineProfile *profile() const;
    QUrl startUrl() const;
    bool isReady() const;

    Q_INVOKABLE void handleNavigation(QWebEngineNavigationRequest *request) const;
    // Returns true if the view should load the address itself. Otherwise the
    // address has been sent to the browser, or blocked.
    Q_INVOKABLE bool handleNewWindow(QWebEngineNewWindowRequest *request) const;
    Q_INVOKABLE void handlePermission(const QWebEnginePermission &permission) const;

Q_SIGNALS:
    void readyChanged();

private:
    void onSecretReady(const QByteArray &secret);
    void onSecretUnavailable(const QString &reason);
    void setReady();

    QQuickWebEngineProfile *m_profile;
    CookieVault *m_cookieVault;
    SecretPortal *m_secretPortal;
    bool m_ready = false;
};
