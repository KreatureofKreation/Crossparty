// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "InviteWatch.h"

namespace
{

constexpr char reportPrefix[] = "crossparty:invites=";
constexpr char unknownCount[] = "none";
// Far more than anyone gets; anything bigger is not a real report.
constexpr int largestCount = 999;

// Read-only: it never changes the page, clicks anything or sends anything
// anywhere. It reports only a number, never names or other page text.
// Microsoft can rename the button at any time; then the reports stop and the
// toasts quietly stop with them, with nothing else affected.
constexpr char watcherSource[] = R"JS(
(() => {
  "use strict";
  const prefix = "crossparty:invites=";
  // Wider than this, the button exists but never shows the invite label, so
  // its count means nothing.
  const narrowestWide = 481;
  // A value is reported only once it has held this long. The label takes a
  // few seconds to appear after the page changes layout, and can flicker;
  // without this, an invite already waiting looked like a new one.
  const settleMs = 5000;
  let reported = null;
  let pending = null;
  let timer = 0;
  const current = () => {
    const guide = document.getElementById("nav-guide");
    if (!guide || window.innerWidth >= narrowestWide) {
      return "none";
    }
    const match = /(\d+)\s+invites?\b/i.exec(guide.textContent || "");
    return match ? String(Number(match[1])) : "0";
  };
  const read = () => {
    const value = current();
    if (value === pending) {
      return;
    }
    pending = value;
    clearTimeout(timer);
    timer = setTimeout(() => {
      if (pending !== reported) {
        reported = pending;
        console.info(prefix + reported);
      }
    }, settleMs);
  };
  new MutationObserver(read).observe(document.documentElement, {
    childList: true,
    subtree: true,
    characterData: true,
  });
  window.addEventListener("resize", read);
  read();
})();
)JS";
}

QWebEngineScript InviteWatch::script()
{
    QWebEngineScript script;
    script.setName(QStringLiteral("crossparty-invite-watch"));
    script.setSourceCode(QString::fromLatin1(watcherSource));
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setWorldId(QWebEngineScript::ApplicationWorld);
    // The main page only, never frames from other sites.
    script.setRunsOnSubFrames(false);
    return script;
}

bool InviteWatch::handleReport(const QString &message)
{
    if (!message.startsWith(QLatin1String(reportPrefix))) {
        return false;
    }
    const QStringView value = QStringView(message).mid(qsizetype(sizeof(reportPrefix) - 1));
    if (value == QLatin1String(unknownCount)) {
        m_count.reset();
        return false;
    }
    bool isNumber = false;
    const int count = value.toInt(&isNumber);
    if (!isNumber || count < 0 || count > largestCount) {
        return false;
    }
    const bool rose = m_count.has_value() && count > *m_count;
    m_count = count;
    return rose;
}
