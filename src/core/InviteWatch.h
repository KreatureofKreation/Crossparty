// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#pragma once

#include <QString>
#include <QWebEngineScript>

#include <optional>

// Notices new party invites, so the app can show its own toast while you are
// in a game. xbox.com has no browser notifications: the only sign of an invite
// is the "1 invite" label on the page's Guide button, and only in its narrow
// layout. A small read-only script watches that label and reports the count
// as a console message; this class decides when the count means a new
// invite. See docs/security.md#sec-017.
class InviteWatch
{
public:
    // Makes a new random report token for this run of the app.
    InviteWatch();

    // The watcher script, run in Chromium's isolated world: it can read the
    // page but not reach the page's own code, and the page cannot reach it.
    // The script carries this run's report token.
    QWebEngineScript script() const;

    // What every genuine report starts with. The random token in it is known
    // only to the watcher script, so a frame or page script that writes a
    // look-alike console message cannot fake or hide an invite.
    QString reportPrefix() const;

    // Reads one console message from the page. Returns true only when the
    // message is a watcher report and the invite count has gone up since the
    // last report. Any other message is ignored.
    bool handleReport(const QString &message);

private:
    // Unknown until the Guide button is seen. While it is missing (the page
    // is in its wide layout), the count is unknown again, and the next count
    // seen is only a starting point, so an invite the user already knew about
    // never raises a toast twice.
    std::optional<int> m_count;
    QString m_reportPrefix;
};
