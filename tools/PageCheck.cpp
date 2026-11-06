// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

// The weekly page check (docs/plan.md, Scheduled checks). Loads Microsoft's
// page signed out, the way the app does, and checks the two things the app
// relies on that Microsoft can change at any time:
//
// 1. Every address the page itself goes to is on the site allowlist, so the
//    app would not be sending the user to the browser mid-sign-in.
// 2. The narrow layout still has the Guide button the invite watcher reads
//    (docs/security.md#sec-017).
//
// It needs the network, so it is never part of the unit tests. It keeps
// nothing: the profile is in-memory, and it never signs in.

#include "SiteAllowlist.h"

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QTimer>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>

#include <cstdlib>

namespace
{

// The overlay's page width (OverlayWindow.qml): narrow enough for the layout
// that shows the invite label.
constexpr int narrowWidth = 440;
constexpr int pageHeight = 720;
// The page builds itself with scripts, and may never report that it has
// finished loading, so it is looked at every few seconds instead.
constexpr int lookEveryMilliseconds = 3000;
constexpr int giveUpMilliseconds = 120000;
// Enough of the page's text to tell an error or block page from the app.
constexpr int textSnippetLength = 300;

void report(const QString &line)
{
    QTextStream(stdout) << line << Qt::endl;
}

// Where an address points, without its query, which can carry tokens.
QString withoutQuery(const QUrl &url)
{
    return url.adjusted(QUrl::RemoveQuery | QUrl::RemoveFragment | QUrl::RemoveUserInfo).toString();
}

// Records where the page goes, and lets it go there: this check only
// reports, it does not enforce.
class RecordingPage : public QWebEnginePage
{
public:
    using QWebEnginePage::QWebEnginePage;

    QStringList hostsOutsideAllowlist;

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override
    {
        Q_UNUSED(type);
        if (isMainFrame) {
            report(QStringLiteral("Going to: ") + withoutQuery(url));
            if (!SiteAllowlist::isAllowedInApp(url) && !hostsOutsideAllowlist.contains(url.host())) {
                hostsOutsideAllowlist.append(url.host());
            }
        }
        return true;
    }
};

// What the page shows right now, as JSON.
QString pageStateScript()
{
    return QStringLiteral(
               "JSON.stringify({width: window.innerWidth, guide: !!document.getElementById('nav-guide'),"
               " title: document.title, text: (document.body ? document.body.innerText : '').slice(0, %1)})")
        .arg(textSnippetLength);
}

}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWebEngineProfile profile;
    RecordingPage page(&profile);
    QWebEngineView view;
    view.setPage(&page);
    view.resize(narrowWidth, pageHeight);
    view.show();

    int lastProgress = -1;
    QObject::connect(&page, &QWebEnginePage::loadProgress, &app, [&lastProgress](int progress) {
        // Every quarter is enough to see where a load stalls.
        if (progress / 25 != lastProgress / 25) {
            report(QStringLiteral("Loading: %1%").arg(progress));
        }
        lastProgress = progress;
    });
    QObject::connect(&page, &QWebEnginePage::loadFinished, &app, [](bool loaded) {
        report(loaded ? QStringLiteral("Load finished") : QStringLiteral("Load failed"));
    });
    QObject::connect(&page, &QWebEnginePage::renderProcessTerminated, &app, [](QWebEnginePage::RenderProcessTerminationStatus status, int code) {
        report(QStringLiteral("FAIL: the page's renderer stopped (status %1, code %2)").arg(int(status)).arg(code));
        QCoreApplication::exit(EXIT_FAILURE);
    });

    // Decides once the Guide button is there, or when time runs out. Answers
    // already on their way are ignored after that.
    bool decided = false;
    auto finish = [&page, &decided](const QJsonObject &state, bool timedOut) {
        if (decided) {
            return;
        }
        decided = true;
        report(QStringLiteral("Page: ") + withoutQuery(page.url()));
        report(QStringLiteral("Title: ") + state.value(QStringLiteral("title")).toString());
        report(QStringLiteral("Width: %1").arg(state.value(QStringLiteral("width")).toInt()));
        bool passed = true;
        if (!page.hostsOutsideAllowlist.isEmpty()) {
            report(QStringLiteral("FAIL: the page went to hosts outside the allowlist: ") + page.hostsOutsideAllowlist.join(QStringLiteral(", ")));
            passed = false;
        }
        if (timedOut) {
            report(
                QStringLiteral("FAIL: no nav-guide button after %1 seconds; invite alerts will stop if this is the real page").arg(giveUpMilliseconds / 1000));
            report(QStringLiteral("The page shows: ") + state.value(QStringLiteral("text")).toString().simplified());
            passed = false;
        }
        if (passed) {
            report(QStringLiteral("PASS"));
        }
        QCoreApplication::exit(passed ? EXIT_SUCCESS : EXIT_FAILURE);
    };

    QTimer look;
    look.setInterval(lookEveryMilliseconds);
    int waitedMilliseconds = 0;
    QObject::connect(&look, &QTimer::timeout, &app, [&] {
        waitedMilliseconds += lookEveryMilliseconds;
        const bool timedOut = waitedMilliseconds >= giveUpMilliseconds;
        page.runJavaScript(pageStateScript(), [&finish, timedOut](const QVariant &result) {
            const QJsonObject state = QJsonDocument::fromJson(result.toString().toUtf8()).object();
            if (state.value(QStringLiteral("guide")).toBool() || timedOut) {
                finish(state, timedOut);
            }
        });
        if (timedOut) {
            look.stop();
        }
    });
    look.start();

    page.load(SiteAllowlist::startUrl());
    return QApplication::exec();
}
