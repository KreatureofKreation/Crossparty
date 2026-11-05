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

public:
    explicit WebSession(QObject *parent = nullptr);

    QQuickWebEngineProfile *profile() const;
    QUrl startUrl() const;

    Q_INVOKABLE void handleNavigation(QWebEngineNavigationRequest *request) const;
    // Returns true if the view should load the address itself. Otherwise the
    // address has been sent to the browser, or blocked.
    Q_INVOKABLE bool handleNewWindow(QWebEngineNewWindowRequest *request) const;
    Q_INVOKABLE void handlePermission(const QWebEnginePermission &permission) const;

private:
    QQuickWebEngineProfile *m_profile;
};
