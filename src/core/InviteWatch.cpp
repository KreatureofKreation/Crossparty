// SPDX-FileCopyrightText: 2026 KreatureofKreation
// SPDX-License-Identifier: MIT

#include "InviteWatch.h"

#include <QRandomGenerator>

#include <array>

namespace
{

constexpr char unknownCount[] = "none";
// Where the script source gets this run's report prefix.
constexpr char prefixPlaceholder[] = "CROSSPARTY_REPORT_PREFIX";
// 128 random bits: far too many to guess.
constexpr int tokenWords = 4;
// Far more than anyone gets; anything bigger is not a real report.
constexpr int largestCount = 999;

// Read-only: it never changes the page, clicks anything or sends anything
// anywhere. It reports only a number, never names or other page text.
// Microsoft can rename the button at any time; then the reports stop and the
// toasts quietly stop with them, with nothing else affected.
constexpr char watcherSource[] = R"JS(
(() => {
  "use strict";
  const prefix = "CROSSPARTY_REPORT_PREFIX";
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

InviteWatch::InviteWatch()
{
    std::array<quint32, tokenWords> token{};
    QRandomGenerator::system()->fillRange(token.data(), token.size());
    QString hex;
    for (const quint32 word : token) {
        hex += QStringLiteral("%1").arg(word, 8, 16, QLatin1Char('0'));
    }
    m_reportPrefix = QStringLiteral("crossparty:") + hex + QStringLiteral(":invites=");
}

QString InviteWatch::reportPrefix() const
{
    return m_reportPrefix;
}

QWebEngineScript InviteWatch::script() const
{
    QWebEngineScript script;
    script.setName(QStringLiteral("crossparty-invite-watch"));
    script.setSourceCode(QString::fromLatin1(watcherSource).replace(QLatin1String(prefixPlaceholder), m_reportPrefix));
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setWorldId(QWebEngineScript::ApplicationWorld);
    // The main page only, never frames from other sites.
    script.setRunsOnSubFrames(false);
    return script;
}

bool InviteWatch::handleReport(const QString &message)
{
    if (!message.startsWith(m_reportPrefix)) {
        return false;
    }
    const QStringView value = QStringView(message).mid(m_reportPrefix.size());
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
