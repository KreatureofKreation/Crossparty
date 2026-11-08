// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "WebSession.h"

#include "CookieVault.h"
#include "NavigationPolicy.h"
#include "PermissionPolicy.h"
#include "SecretPortal.h"
#include "SiteAllowlist.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QLoggingCategory>
#include <QQuickWebEngineDownloadRequest>
#include <QStandardPaths>
#include <QTimer>
#include <QWebEngineCookieStore>

Q_LOGGING_CATEGORY(lcWeb, "crossparty.web", QtInfoMsg)
// Microsoft's own page logs: mostly telemetry noise, so hidden unless asked
// for with QT_LOGGING_RULES="crossparty.page.debug=true".
Q_LOGGING_CATEGORY(lcPage, "crossparty.page", QtInfoMsg)

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

// Inside Flatpak this is the app's own data folder, which no other app can
// read.
QString cookieFilePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/cookies.vault");
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
    , m_cookieVault(new CookieVault(m_profile->cookieStore(), cookieFilePath(), this))
    , m_secretPortal(new SecretPortal(this))
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

    // Invite and message alerts arrive as the page's own browser
    // notifications (SEC-011 allows them for Microsoft's sites only).
    connect(m_profile, &QQuickWebEngineProfile::presentNotification, this, &WebSession::onNotification);

    // Save any last change before the app exits.
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, m_cookieVault, &CookieVault::saveNow);

    connect(m_secretPortal, &SecretPortal::secretReady, this, &WebSession::onSecretReady);
    connect(m_secretPortal, &SecretPortal::secretUnavailable, this, &WebSession::onSecretUnavailable);
    m_secretPortal->requestSecret();
}

void WebSession::onNotification(QWebEngineNotification *notification)
{
    // Defence in depth: the permission policy already refuses notifications
    // to other sites.
    if (!SiteAllowlist::isAllowedInApp(notification->origin())) {
        notification->close();
        return;
    }
    // Tells the page its notification was shown. Its text is never logged.
    notification->show();
    Q_EMIT notificationArrived(notification->title(), notification->message());
}

void WebSession::signOutAndWipe()
{
    // The saved sign-in first, so nothing below can be saved again.
    const bool fileDeleted = m_cookieVault->wipe();
    m_profile->cookieStore()->deleteAllCookies();
    m_profile->clearHttpCache();
    if (!fileDeleted) {
        // Never claim a wipe that did not happen: stay open and say so.
        qCWarning(lcWeb) << "Signed out, but the saved sign-in could not be deleted";
        Q_EMIT wipeFailed(cookieFilePath());
        return;
    }
    qCInfo(lcWeb) << "Signed out and wiped; closing";
    // Everything else the page stored (local storage, databases, service
    // workers) is held only in this process's memory, and goes with it.
    QTimer::singleShot(0, QCoreApplication::instance(), &QCoreApplication::quit);
}

void WebSession::handleConsoleMessage(const QString &message, const QUrl &pageUrl)
{
    // Only Microsoft's own pages can raise a toast (SEC-010). A report can
    // only ever say "the count went up", never put page text on screen.
    if (SiteAllowlist::isAllowedInApp(pageUrl) && m_inviteWatch.handleReport(message)) {
        qCInfo(lcWeb) << "New party invite";
        Q_EMIT inviteArrived();
        return;
    }
    qCDebug(lcPage) << message;
}

QWebEngineScript WebSession::inviteWatchScript() const
{
    return m_inviteWatch.script();
}

void WebSession::onSecretReady(const QByteArray &secret)
{
    m_cookieVault->unlockAndRestore(secret);
    setReady();
}

void WebSession::onSecretUnavailable(const QString &reason)
{
    // Fail safe: without a key, sign-in is simply not saved. The app still
    // works; the user signs in again next time.
    qCWarning(lcWeb) << "No key for saved sign-in:" << reason;
    setReady();
}

void WebSession::setReady()
{
    if (!m_ready) {
        m_ready = true;
        Q_EMIT readyChanged();
    }
}

bool WebSession::isReady() const
{
    return m_ready;
}

QQuickWebEngineProfile *WebSession::profile() const
{
    return m_profile;
}

QUrl WebSession::startUrl() const
{
    return SiteAllowlist::startUrl();
}

void WebSession::handleNavigation(QWebEngineNavigationRequest *request) const
{
    const QUrl url = request->url();
    // Going back or forward within Microsoft's app never comes here: its
    // pages change without loading a new document. One that does come here
    // would load a whole page, which ends the party, so it is refused. This
    // covers a mouse's Back button and the controller's B (GamepadInput).
    if (request->isMainFrame() && request->navigationType() == QWebEngineNavigationRequest::BackForwardNavigation) {
        request->reject();
        qCInfo(lcWeb) << "Back or forward would reload the party page; refused:" << describeForLog(url);
        return;
    }
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

void WebSession::handleNewWindow(QWebEngineNewWindowRequest *request) const
{
    const QUrl url = request->requestedUrl();

    switch (NavigationPolicy::decideForNewWindow(url)) {
    case NavigationPolicy::Decision::OpenInBrowser:
        openInBrowser(url);
        return;
    case NavigationPolicy::Decision::LoadInApp:
    case NavigationPolicy::Decision::Block:
        qCInfo(lcWeb) << "New window blocked:" << describeForLog(url);
        return;
    }
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
