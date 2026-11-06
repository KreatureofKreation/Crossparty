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
// The page builds itself with scripts after loading; give it time.
constexpr int settleMilliseconds = 20000;
constexpr int giveUpMilliseconds = 120000;

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
        if (isMainFrame && !SiteAllowlist::isAllowedInApp(url) && !hostsOutsideAllowlist.contains(url.host())) {
            hostsOutsideAllowlist.append(url.host());
        }
        return true;
    }
};

void report(const QString &line)
{
    QTextStream(stdout) << line << Qt::endl;
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

    int exitCode = EXIT_FAILURE;

    QTimer::singleShot(giveUpMilliseconds, &app, [] {
        report(QStringLiteral("FAIL: the page did not finish loading in time"));
        QCoreApplication::exit(EXIT_FAILURE);
    });

    QObject::connect(&page, &QWebEnginePage::loadFinished, &app, [&](bool loaded) {
        if (!loaded) {
            report(QStringLiteral("FAIL: the page did not load"));
            QCoreApplication::exit(EXIT_FAILURE);
            return;
        }
        QTimer::singleShot(settleMilliseconds, &app, [&] {
            const QString script = QStringLiteral("JSON.stringify({width: window.innerWidth, guide: !!document.getElementById('nav-guide')})");
            page.runJavaScript(script, [&](const QVariant &result) {
                const QString state = result.toString();
                const bool hasGuide = state.contains(QLatin1String("\"guide\":true"));
                report(QStringLiteral("Page: ") + page.url().host());
                report(QStringLiteral("Layout: ") + state);
                bool passed = true;
                if (!page.hostsOutsideAllowlist.isEmpty()) {
                    report(QStringLiteral("FAIL: the page went to hosts outside the allowlist: ") + page.hostsOutsideAllowlist.join(QStringLiteral(", ")));
                    passed = false;
                }
                if (!hasGuide) {
                    report(QStringLiteral("FAIL: the narrow layout has no nav-guide button; invite alerts will stop"));
                    passed = false;
                }
                if (passed) {
                    report(QStringLiteral("PASS"));
                }
                exitCode = passed ? EXIT_SUCCESS : EXIT_FAILURE;
                QCoreApplication::exit(exitCode);
            });
        });
    });

    page.load(SiteAllowlist::startUrl());
    return QApplication::exec();
}
