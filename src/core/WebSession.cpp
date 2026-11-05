// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "WebSession.h"

#include "NavigationPolicy.h"
#include "PermissionPolicy.h"

#include <QDesktopServices>
#include <QLoggingCategory>
#include <QQuickWebEngineDownloadRequest>

Q_LOGGING_CATEGORY(lcWeb, "crossparty.web", QtInfoMsg)

namespace
{

// Logs never contain full addresses, which can carry personal details or
// sign-in tokens. The scheme and host are enough to see what happened.
QString describeForLog(const QUrl &url)
{
    return url.scheme() + QStringLiteral("://") + url.host();
}

void openInBrowser(const QUrl &url)
{
    // Inside Flatpak this goes through the OpenURI portal, so the user's
    // default browser opens the link outside our sandbox.
    qCInfo(lcWeb) << "Opening in the default browser:" << describeForLog(url);
    QDesktopServices::openUrl(url);
}
}

WebSession::WebSession(QObject *parent)
    : QObject(parent)
    // A profile created without a storage name is in-memory only: Chromium
    // writes no cookies, Local Storage or cache to disk. See
    // docs/security.md#sec-009.
    //
    // Qt 6.11 logs that QML code should use WebEngineProfilePrototype instead.
    // That type has no C++ API, and creating the profile here keeps the
    // in-memory guarantee in the core, so the warning is expected for now.
    , m_profile(new QQuickWebEngineProfile(this))
{
    if (!m_profile->isOffTheRecord()) {
        // Fail safe: never run with a profile that would write session data
        // to disk unencrypted.
        qFatal("The web profile is not in-memory; refusing to continue");
    }

    // The app has no access to your files (no file system permission), and a
    // party chat app never needs to download anything.
    connect(m_profile, &QQuickWebEngineProfile::downloadRequested, this, [](QQuickWebEngineDownloadRequest *download) {
        qCInfo(lcWeb) << "Download refused";
        download->cancel();
    });
}

QQuickWebEngineProfile *WebSession::profile() const
{
    return m_profile;
}

QUrl WebSession::startUrl() const
{
    return QUrl(QStringLiteral("https://www.xbox.com/play"));
}

void WebSession::handleNavigation(QWebEngineNavigationRequest *request) const
{
    const QUrl url = request->url();
    const NavigationPolicy::Decision decision = request->isMainFrame() ? NavigationPolicy::decideForPage(url) : NavigationPolicy::decideForEmbeddedFrame(url);

    switch (decision) {
    case NavigationPolicy::Decision::LoadInApp:
        request->accept();
        return;
    case NavigationPolicy::Decision::OpenInBrowser:
        request->reject();
        openInBrowser(url);
        return;
    case NavigationPolicy::Decision::Block:
        request->reject();
        qCInfo(lcWeb) << "Navigation blocked:" << describeForLog(url);
        return;
    }
}

bool WebSession::handleNewWindow(QWebEngineNewWindowRequest *request) const
{
    const QUrl url = request->requestedUrl();

    switch (NavigationPolicy::decideForPage(url)) {
    case NavigationPolicy::Decision::LoadInApp:
        return true;
    case NavigationPolicy::Decision::OpenInBrowser:
        openInBrowser(url);
        return false;
    case NavigationPolicy::Decision::Block:
        qCInfo(lcWeb) << "New window blocked:" << describeForLog(url);
        return false;
    }
    return false;
}

void WebSession::handlePermission(const QWebEnginePermission &permission) const
{
    const bool allowed = PermissionPolicy::isAllowed(permission.permissionType(), permission.origin());
    qCInfo(lcWeb) << "Permission" << permission.permissionType() << "for" << describeForLog(permission.origin()) << (allowed ? "granted" : "denied");
    if (allowed) {
        permission.grant();
    } else {
        permission.deny();
    }
}
