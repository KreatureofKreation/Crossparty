# Crossparty: project plan

This file is the single source of truth for the project. It started as a PDF
("Crosstalk — Project Plan", 4 November 2026) and moved into the repo on
5 November 2026, so changes are reviewed and kept in git history like code.
Amendments since the PDF are marked _(amended DATE)_.

## Overview

Crossparty is an unofficial, native-feeling Linux app for Xbox party chat on
the web, with a normal window, a summon-on-hotkey in-game overlay, and a
handheld mode. Status: stage 1 (the spike) is done and confirmed Qt; stage 2
(window mode) is next. _(amended 2026-11-05)_

- **Tagline:** Cross-platform party chat for Linux
- **Store description:** An unofficial desktop app for Xbox party chat on the
  web.
- **Primary target:** Bazzite (KDE Plasma 6, Wayland) desktop, plus handhelds
  in Game Mode.
- **Goals:** a daily-driver app that feels native, light and safe; and a
  learning and portfolio project for a cybersecurity career, with every
  security decision explained and documented.
- **Non-goal:** reimplementing Microsoft's private voice protocol. Crossparty
  wraps the official web app at xbox.com/play.

## Decisions log

Every decision below is locked unless the spike disproves it. Any change of
plan is recorded here before it is built.

| Decision | Choice | Why |
|---|---|---|
| Approach | Wrap xbox.com/play, no protocol reverse-engineering | Private protocol, ToS and ban risk, breaks on every update |
| Name | Crossparty _(amended 2026-11-04)_ | Was Crosstalk. The trademark check found live USPTO "Crosstalk" marks in software (classes 9 and 42); "Crossparty" had none in IP Australia or the USPTO |
| App ID | `io.github.KreatureofKreation.Crossparty` _(amended 2026-11-04)_ | Flathub format; permanent once published |
| UI toolkit | Qt 6 + QML + QtWebEngine + LayerShellQt | Lighter than Electron, native KDE look, real Wayland overlay. Confirmed by the spike _(amended 2026-11-05)_ |
| Fallback toolkit | Electron | Used only if party voice fails in QtWebEngine. Not needed: voice passed in the spike _(amended 2026-11-05)_ |
| Display | Native Wayland, no X11 | X11 apps can read other apps' keystrokes, including games on XWayland |
| Packaging | Flatpak from day one | Outer sandbox, clear permissions, native on Bazzite |
| Distribution | Flathub stable + beta; signed own repo as fallback | Verified updates and permission review |
| IaC tool | OpenTofu | Same language as Terraform, open licence, encrypted state built in |
| Dev environment | Distrobox container defined in the repo | Keeps tools off the immutable host; reproducible |
| Overlay | Hidden until summoned, top-right, opens with focus | Used for invites, audio, achievements, then dismissed |
| Overlay dismiss | Hotkey or the top-bar button _(amended 2026-11-05: Esc and click outside dropped, see Overlay keyboard)_ | Matches Steam overlay habits |
| Hotkeys | Meta+Shift+X overlay, Meta+Shift+M mute | Mute works everywhere; via KDE's shortcut portal |
| Click-through | Cut | Overlay is summoned with focus, so not needed |
| Official support | Plasma 6 Wayland + Gamescope | What can actually be tested; others best effort |
| Learning mode | On | Plain-English comments, explained decisions, security write-up |
| Cookie storage | In-memory web profile; the app saves only cookies, encrypted _(amended 2026-11-05)_ | QtWebEngine never encrypts its cookie database (spike test 8). Costs one click on Sign in per launch. See SEC-009 |
| Cookie key | From the XDG Secret portal (KWallet), encrypted by the app with AES-256-GCM _(amended 2026-11-05)_ | Needs no Flatpak permission. Talking to Secret Service directly would open far more of the keyring than the app needs |
| Translation | Qt's own system (`qsTr`, Qt Linguist), not KDE's KI18n _(added 2026-11-05)_ | No extra library, and the app is not part of KDE's translation infrastructure |
| Microphone mute | Mute the app's own recording stream in PipeWire, found by the app ID PipeWire tags it with _(added 2026-11-05)_ | QtWebEngine only mutes page output, and scripting the page is ruled out. Tested: the page hears silence. See SEC-014 |
| Overlay keyboard | Mouse-only: the overlay never takes the keyboard, and opens and closes by hotkey or the top-bar button _(added 2026-11-05)_ | Games minimise or leave full screen when they lose the keyboard (tested). Replaces "opens with focus" and Esc and click-outside dismiss |
| Overlay fit | Page zoom 90%, page width 440 CSS px, so xbox.com shows its narrow layout with the party panel filling the overlay _(added 2026-11-05)_ | Uses the page's own layout; nothing is cropped or changed. Same fit at any resolution or scaling |
| Overlay content | The same live page moves between the window and the overlay _(added 2026-11-05)_ | Keeps one party connection. Tested: the party stayed connected both ways, over a full-screen game |
| Window position | Remember size only; position is left to KDE _(amended 2026-11-05)_ | Wayland does not let apps place their own windows. A KWin window rule can remember position; documented in the README |
| Invite toasts | A read-only watcher script, in Chromium's isolated world, reads the invite count on the page's Guide button. While the window is hidden or minimised, the page is laid out at the overlay's narrow width so that count is shown _(added 2026-11-06)_ | xbox.com sends no browser notifications and has no pop-ups of its own in a browser; the narrow layout's "1 invite" label is the only signal (tested). The one exception to "no scripting": it reads one label and reports a number. Message alerts are dropped: their only counter is inside the Guide, and opening it clears them. The start page moves to play.xbox.com, Microsoft's newer app, because only it has the invite count. See SEC-017 |
| Handheld mode | Detected from Gamescope's environment; full screen with a handheld bar (Mute, Sign Out and Wipe); no overlay, corner notices or invite toasts _(added 2026-11-06)_ | Gamescope has no layer above games, and Steam's own menu already switches between apps. Gamescope offers only X11, so X11 is allowed as a fallback when no Wayland is running. See SEC-019 |
| Sign out and wipe | Erases the saved sign-in file, cookies and cache, then closes the app _(added 2026-11-06)_ | Page storage (local storage, databases) lives only in the in-memory profile, and QtWebEngine has no call to clear it, so closing is the only complete wipe. One confirmation, Cancel by default. See SEC-018 |

**Priority (decided 4 Nov):** private use first; public distribution is a
bonus. Security, sandboxing, clean code and the Flatpak build stay in scope
regardless. Publish-only work (Flathub submission, AppStream metadata, public
disclaimers, the fallback signed repo) waits until going public is decided.
The trademark check was done early, on 4 Nov. _(amended 2026-11-05)_

## Features

v1 is the smallest app you would actually use every day; everything else waits
until it is solid.

### v1

- Persistent sign-in (log in once; one click on Sign in per launch, see the
  cookie storage decision) _(amended 2026-11-05)_
- Mic, speaker and notification access granted to Microsoft's sites only
- Single instance: reopening brings back the running window, never a second
  party connection
- Window mode with a minimal native top bar
- Closing sends it to the tray; Quit lives in the tray menu
- Overlay: hidden until summoned, top-right, opens with focus, dismissed by
  hotkey, Esc or click outside
- Overlay zoom so Microsoft's page fits the compact panel
- Mute hotkey that works everywhere
- Muted badge: a small corner icon shown only while muted
- Invite alerts, read from the page's own invite count _(amended 2026-11-06:
  the page sends no browser notifications, and message alerts are not
  possible, see Invite toasts)_
- Handheld mode: full screen and controller-friendly when Game Mode is detected
- Remembers each mode's size _(amended 2026-11-05: size only, see Window
  position)_
- Sign out and wipe: clears all session data in one click

### Later

- Corner and opacity presets for the overlay and badge
- Autostart on login
- Achievements shortcut button (achievements already work inside the page)
- Push-to-talk. The spike showed the portal does report key release, so this
  is possible; it stays here until v0.1.0 _(amended 2026-11-05)_
- "You're talking while muted" warning
- A "Type a message" button in the overlay that hands it the keyboard on
  request, accepting that some games minimise then _(added 2026-11-05)_
- Hotkey feedback: a short notification ("Microphone muted" / "Microphone
  on") when a hotkey changes something, through the notification portal, shown
  in the overlay over full-screen games. Raised during the spike; the muted
  badge covers the mute state in v1 _(added 2026-11-05)_

### Cut

| Feature | Why it is cut |
|---|---|
| Native voice protocol | Private, fragile, risks your account |
| Click-through overlay | Overlay is summoned with focus instead |
| Audio device picker | KDE already does it per app; a sandboxed app should not change system audio |
| Talkers-only slim overlay | Needs reading Microsoft's page layout, which breaks on every redesign |
| Cloud gaming features | Works for free inside the page; designing for it turns a chat app into a streaming app |

## UX and design

Crossparty should look like it shipped with Plasma: no custom theme, minimal
chrome, and Microsoft's page as the content.

### Window mode

- A top bar about 36 px tall: mute, overlay, minimise, maximise, close.
- Follows the system theme, accent colour, font and light/dark mode through
  Kirigami.
- Icons from the system icon theme.

### Overlay

- Summoned with Meta+Shift+X, top-right. Mouse-only: it never takes the
  keyboard, so games keep focus _(amended 2026-11-05)_.
- Dismissed with the same hotkey or the top-bar button _(amended
  2026-11-05)_.
- Page zoomed to 90% and 440 CSS px wide, so xbox.com's own narrow layout
  fills the panel with the party view _(amended 2026-11-05)_.
- Controller input still reaches the game while the overlay is open, so use
  mouse and keyboard in it on desktop.
- Works over borderless games. Over exclusive full-screen games, taking the
  keyboard drops the game back to windowed (spike test 4); on-demand keyboard
  focus is tried in stage 3 _(amended 2026-11-05)_

### Always visible

- Muted badge in the top-right corner, shown only while muted. While the
  overlay is open, its own status bar shows the microphone state instead
  _(amended 2026-11-05)_.
- Badge and toasts never take the keyboard and let clicks through to the game
  _(added 2026-11-05)_.
- Invite alerts as small corner toasts naming the hotkey. They say only that
  an invite is waiting, never who sent it _(amended 2026-11-06)_.

### Handheld mode

- Full screen, larger touch and controller targets, thick focus outlines.
- Switched to with the Steam button; voice keeps running in the background.
- Controller through Steam Input: the D-pad as arrow keys and A as Enter on
  the handheld bar, and Down from the bar goes to the page. A template that
  also gives a mouse (trackpad or stick) works best for the page itself
  _(added 2026-11-06)_.
- No overlay, muted badge or invite toasts: Gamescope has no layer above
  games for them _(added 2026-11-06)_.
- Detected from Gamescope's environment; `CROSSPARTY_HANDHELD=1` forces it on
  and `=0` off _(added 2026-11-06)_.

### Rules that keep it from looking AI-made

- No purple or indigo gradients, frosted glass, emoji icons, floating rounded
  cards, or Inter font.
- No custom brand colours in the app UI; an icon may use a muted jade.
- Animations under 150 ms, only to show a change of state.
- No pop-up dialogs except the Sign out and wipe confirmation.
- Full keyboard navigation and screen reader labels on every control.

### Performance budgets (measured, not guessed)

| Measure | Target |
|---|---|
| Launch to window visible | under 1 s |
| Overlay show or hide | under 100 ms |
| CPU idle in tray | near zero |
| Crossparty's own memory | measured separately from Microsoft's page and kept as low as possible |

User feel is tuned through testing from build step 2 onward.

## Architecture and stack

Three parts run inside the Flatpak sandbox, and the C++ core decides what loads
in the app and what leaves it.

The interface asks the core for everything; the core talks to KDE only through
portals and D-Bus, sends Microsoft's sites to QtWebEngine, and hands every
other link to your default browser.

- **Interface (QML):** window mode, overlay and handheld layouts; top bar,
  muted badge, toasts.
- **Core (C++):** site allowlist, permission handler, settings, mute state,
  single instance, Game Mode detection, encrypted cookie storage _(amended
  2026-11-05)_.
- **QtWebEngine:** displays xbox.com/play. Chromium's own sandbox runs inside
  Flatpak (spike test 7) _(amended 2026-11-05)_.
- **LayerShellQt:** places the overlay and badge on a layer above games. Not in
  the KDE runtime, so the manifest builds it _(amended 2026-11-05)_.

## Security and threat model

The biggest risks are a stolen Microsoft session, a hijacked mic, and a
tampered download; the main attack path is a link in party text chat.

### Assets

| Asset | Why it matters |
|---|---|
| Microsoft session cookies | Whoever holds them is logged in as you |
| Microphone | Live audio from your room |
| The release download | A swapped copy runs on every user's machine |
| GitHub account, OpenTofu token, signing key | Control of the code, settings and updates |

### Threats and controls

| Threat | Control |
|---|---|
| Malicious link in party chat | Site allowlist: only xbox.com and Microsoft login load in-app; everything else opens in the default browser, https only |
| Page escapes into the system | Flatpak sandbox around the whole app, plus Chromium's own sandbox, confirmed running inside Flatpak _(amended 2026-11-05)_; no native bridges exposed to the page |
| Over-granted permissions | Deny by default; mic, speakers and notifications for allowlisted origins only |
| Cookie theft from disk | Web data kept in memory; cookies saved encrypted with a key from the Secret portal (KWallet) _(amended 2026-11-05)_ |
| Keystroke snooping | Native Wayland only; no X11 socket |
| Stale browser engine | QtWebEngine patched through the KDE runtime; risk accepted because only Microsoft origins load |
| Compromised dependency | Minimal dependencies, pinned versions, offline builds, Dependabot |
| Tampered release | Builds only in CI, build attestation, Flathub or GPG-signed repo |
| Account or key takeover | 2FA with hardware key, token only on the PC, revoke-and-rotate plan in SECURITY.md |

**Known accepted risk:** QtWebEngine receives Chromium security fixes later
than Chrome. Exposure is limited because only Microsoft's own sites ever load.
This is recorded in docs/security.md and reviewed at each release.

## Privacy

Crossparty collects nothing; Microsoft still sees everything said and done in a
party, and that limit is stated in the README.

- No telemetry, analytics or crash reports sent anywhere.
- Settings stored in one local file; session data stays in the Flatpak's own
  folder.
- Logs never contain cookies, tokens, or URLs with personal details.
- The mic is only live when Microsoft's page uses it, and the muted badge and
  top-bar indicator always show its state.
- Sign out and wipe removes all session data in one click.
- A short privacy statement in the README: what is stored, where, and that
  nothing leaves the machine except traffic to Microsoft.

## Packaging and distribution

Crossparty ships only as a Flatpak, built on the KDE runtime with the fewest
permissions that let it work.

### Flatpak permissions

| Permission | Granted | Reason |
|---|---|---|
| `--socket=wayland` | Yes | Display |
| `--socket=pulseaudio` | Yes | Mic and speakers (Flatpak has no separate mic switch) |
| `--share=network` | Yes | Reach Microsoft's servers |
| `--device=dri` | Yes | GPU acceleration |
| Tray (StatusNotifier) | Yes | Tray icon |
| Secret portal | Via portal _(amended 2026-11-05)_ | Key for encrypted cookies; no extra permission needed. Was "Secret Service: Yes" |
| Notifications, global shortcuts | Via portals | No extra permission needed; user approves in KDE |
| X11 socket | No | Keystroke-snooping risk |
| File system or home folder | No | Not needed |
| Other devices | No | Not needed |

### Channels

- Flathub stable: built from tagged releases on main.
- Flathub beta: built from staging, for testing before promotion.
- Fallback if Flathub rejects it: our own Flatpak repo on GitHub Pages,
  GPG-signed, built by CI.

### Flathub readiness

- Neutral name and icon, no Microsoft trademarks or logos.
- Offline build: all sources pinned, no network during build.
- AppStream metadata with screenshots, licence and the unofficial disclaimer.

## IaC and dev environment

Everything that sets up the project lives in the repo as code, so a fresh clone
rebuilds the same setup.

### OpenTofu: GitHub settings as code

- Manages: branch rulesets for main and staging, required CI checks, secret
  scanning with push protection, Dependabot alerts, release tag rules.
- State file encrypted with OpenTofu's built-in client-side encryption; kept
  out of the repo.
- The GitHub admin token is the most sensitive secret in the project:
  short-lived, stored in a password manager, used only on the PC, never in CI.

### Build and CI as code

- GitHub Actions workflows, with every action pinned by commit hash.
- The Flatpak manifest is the build recipe; runtime and every source pinned.

### Dev environment as code

- A container definition in the repo, run with Distrobox (ships with Bazzite).
- Holds Qt 6 dev tools, flatpak-builder, OpenTofu, linters and formatters;
  nothing installed on the immutable host.
- One setup script brings a new machine to a working build.

## Dev practices and CI

Code is written to be read: a beginner should follow any file top to bottom,
and tools enforce the style so reviews focus on logic.

### Clean, readable code

- Follow KDE's coding style for C++ and QML; formatting is automatic
  (clang-format, qmlformat), never argued about.
- Names say what things are: `overlayWindow`, `isMuted`, `allowedOrigins`. No
  abbreviations, no single letters outside short loops.
- Small functions that do one thing; if a function needs a comment to explain
  what it does, split or rename it.
- Comments explain why, not what. Every security decision gets a comment
  linking to its entry in docs/security.md.
- No clever tricks: plain, boring code over short code.
- One responsibility per file: allowlist, permissions, overlay, tray, settings
  each live separately.
- No dead code or commented-out code; git history keeps the old versions.
- Constants named in one place (origins, hotkeys, sizes), never magic values
  scattered through files.
- Errors handled visibly: fail safe (deny, close, log) rather than fail open.

### Tests

- Unit tests for every security-critical piece: allowlist, permission handler,
  link handling, settings parsing, cookie encryption _(amended 2026-11-05)_.
- A written manual test checklist for voice, overlay and handheld (see Testing
  plan).

### Linters and scanners in CI (every push)

- clang-tidy and clazy (Qt-specific checks) for C++; qmllint for QML.
- CodeQL security scanning.
- flatpak-builder-lint to check the manifest against Flathub rules.
- Pre-commit hooks run the formatters and linters locally before each commit.

### Workflow

- Small pull requests into staging, each with a clear description; nothing
  pushed straight to main.
- Commit messages in plain English: what changed and why.
- A short review checklist on every PR: readable, tested, no new permissions,
  no secrets, docs updated.

### Repo files

- README (install, setup, support matrix, disclaimer, privacy statement)
- SECURITY.md (reporting, key-loss plan)
- LICENSE (MIT)
- CONTRIBUTING.md (style, workflow, how to run tests)
- CHANGELOG.md
- docs/security.md (threat model and decision log)
- docs/spike.md (stage 1 results) _(amended 2026-11-05)_
- docs/plan.md (this plan) _(amended 2026-11-05)_

## Release and maintenance

Nothing reaches main (stable) without passing through staging (beta) first,
and scheduled checks catch breakage before users report it.

### Staging vs production

1. All work and all automated updates (Dependabot, runtime bumps) land on
   staging.
2. staging builds are installed locally from CI (Flathub beta once public);
   test them on the PC and, where possible, a handheld.
3. When the manual checklist passes, merge staging into main and tag a release.
4. Tagged releases on main become your installed version (Flathub stable once
   public).
5. A broken stable release is fixed by reverting on staging and re-promoting,
   never by editing main directly.

### Scheduled checks (weekly, in CI)

- Does the narrow layout still have the `nav-guide` button the invite watcher
  reads (SEC-017)? _(added 2026-11-06)_

- Does xbox.com/play still load in the current QtWebEngine with our user
  agent, and do the allowlisted origins still match where it redirects?
- Is there a newer KDE runtime or dependency release waiting?
- Results open a GitHub issue automatically when something fails.

**Limit:** these checks run signed out. A signed-in check would need a
Microsoft account's credentials stored in CI, which breaks the "no secrets in
CI" rule, so voice and party features stay on the manual checklist.

### Versioning

- Semantic versioning: major.minor.patch (patch = fixes, minor = features,
  major = breaking changes).
- Every release gets a CHANGELOG entry in plain English.

### Ongoing

- A monthly look even when checks are green.
- README states it is a hobby project with best-effort fixes.

## Testing plan

The spike has written pass/fail rules, development uses a spare account, and
handheld mode is tested in nested Gamescope on the desktop.

### Accounts and people

- A second Microsoft account for development (never the main gamertag)
- Someone on the other end of the party: a friend, or the main account on a
  phone

### Spike pass/fail rules (decided before running it)

Results are in [spike.md](spike.md). _(amended 2026-11-05)_

| Test | Pass | If it fails | Result |
|---|---|---|---|
| xbox.com/play loads in QtWebEngine | Page loads, no unsupported-browser block | Adjust user agent; still blocked after two sessions → switch to Electron | Provisional pass; stream start not yet checked |
| Sign-in persists | Still signed in after restarting the app | Fix storage; unresolved after two sessions → Electron | Pass |
| Party voice | Two-way audio with the other account for 10 minutes | Unresolved after two sessions → Electron | Pass |
| Overlay as a layer surface | Sits above a borderless game, takes focus, hides on Esc | Fall back to a KWin window rule, documented in README | Pass |
| Global shortcuts portal | KDE approval prompt appears, hotkeys fire in-game | Fall back to tray-only control plus documented KDE custom shortcut | Pass |
| Key release events | Portal reports key release | Push-to-talk stays cut | Pass |
| Chromium sandbox inside Flatpak | QtWebEngine's own sandbox runs alongside Flatpak's | Rely on Flatpak's sandbox alone; update the threat model and docs/security.md | Pass |
| Keyring inside Flatpak | Cookies saved encrypted through Secret Service | Fix before stage 2; never fall back to plain-text storage | Fail; fixed by the cookie storage decision (SEC-009) |

### Handheld testing without a handheld

- Close Crossparty on the desktop first (a second copy only brings the first
  to the front), then run
  `gamescope -W 1280 -H 800 -- flatpak run --nosocket=wayland --socket=x11 io.github.KreatureofKreation.Crossparty`
  to mimic Game Mode. The two overrides are needed only when nested: the
  desktop's own Wayland display stops Flatpak's X11 fallback, which real
  Game Mode does not have _(amended 2026-11-06)_.
- One real-device check on a Deck or Ally before the first public release, if
  one can be borrowed.

### Manual checklist (every promotion from staging to main)

- Fresh install, sign in, restart: still signed in
- Join a party, talk both ways, mute and unmute by hotkey
- Muted badge appears and disappears correctly
- Overlay opens over a borderless and a full-screen game, the game keeps
  focus, and it closes by hotkey _(amended 2026-11-05)_
- Invite toast appears while the overlay is hidden
- A link in party chat opens in the default browser, not in the app
- Sign out and wipe leaves no session data
- Nested Gamescope: handheld mode opens full screen and is
  controller-navigable
- Performance budgets met

## Legal, licences and disclaimers

Crossparty is MIT-licensed, uses no Microsoft branding, and says plainly that
it is unofficial. (Not legal advice; checked against public guidance only.)

- **Licence:** MIT for Crossparty's code.
- **Qt and QtWebEngine:** LGPL; compatible because the Flatpak runtime supplies
  Qt, unmodified and dynamically linked. Stated in the README.
- **Branding:** no Xbox or Microsoft logos, names or colours in the app name,
  icon or UI. The word Xbox appears only to describe what the app works with.
- **Trademark check:** done 4 Nov 2026 in IP Australia and the USPTO. Live
  "Crosstalk" marks in software led to the rename to Crossparty, which had no
  marks in either register. Repeat before anything is published. _(amended
  2026-11-05)_
- **README disclaimer:** "Crossparty is an unofficial project and is not
  affiliated with, endorsed by, or sponsored by Microsoft. Xbox is a trademark
  of Microsoft Corporation."
- **No warranty:** covered by the MIT licence; repeated in plain words in the
  README.
- **Microsoft's terms:** Crossparty only displays Microsoft's own web app, the
  same as a browser. It never automates, scrapes or modifies the page, which
  keeps it on the safe side of their terms.
- **Privacy statement:** in the README (see Privacy).

## Build order

Seven stages; the spike in stage 1 decided that the rest is built on Qt.

| Stage | Work | Status |
|---|---|---|
| 0. Setup | Trademark check; second Microsoft account and a test partner; GitHub 2FA with a hardware key; GitHub username and app ID chosen (permanent); repo, Distrobox dev container, formatters, linters and pre-commit hooks from the first commit; minimal CI (build, lint, unit tests on every push) | Done, except hardware-key 2FA _(amended 2026-11-05)_ |
| 1. Spike (throwaway) | Every test in the spike table run inside a Flatpak build; results in docs/spike.md; confirm Qt or switch to Electron | Done: Qt _(amended 2026-11-05)_ |
| 2. Window mode | Main window, top bar, system theme; site allowlist, permission handler and keyring-encrypted cookies, with unit tests; mode-ready layout (window, overlay and handheld as separate views over one shared core); all interface text wrapped for translation; settings file carries a version number; tray, close-to-tray, single instance, saved size | Done _(amended 2026-11-06)_ |
| 3. Overlay and hotkeys | Layer-surface overlay, focus on open, Esc and click-outside dismiss; shortcuts portal for overlay and mute hotkeys; muted badge and invite toasts; mute the microphone itself (our capture stream, not page output) _(amended 2026-11-05)_ | Done _(amended 2026-11-06)_ |
| 4. Handheld mode | Game Mode detection, full-screen controller layout; tested in nested Gamescope; Sign out and wipe | Done _(amended 2026-11-06)_ |
| 5. Hardening and CI | CodeQL added to CI; docs/security.md threat model written up; Flatpak manifest finalised (offline build, flatpak-builder-lint clean); OpenTofu config for repo settings, encrypted state | Next |
| 6. Packaging and release | staging → Flathub beta, main → Flathub stable (once public); scheduled checks; README, SECURITY.md, CONTRIBUTING.md, CHANGELOG.md, disclaimer; manual checklist passes; tag v0.1.0 | |

### Stage 5 pull requests _(added 2026-11-06)_

1. Flatpak manifest finalised: AppStream metainfo and an app icon,
   flatpak-builder-lint clean on the manifest and the build, run by
   dev/flatpak-build.sh and CI. The build is offline (every source pinned, no
   network while building). Screenshots are left to stage 6, as a recorded
   lint exception, because they need a public URL and a picture with no
   account details in it.
2. CodeQL security scanning in CI: C++ and the workflows, without a build, on
   every push and weekly (SEC-020).
3. The threat model written up in docs/security.md.
4. OpenTofu config for the repo settings, with encrypted state.

### Stage 4 pull requests _(added 2026-11-06)_

1. Sign out and wipe: an options menu in the top bar and a tray menu entry,
   one confirmation, then the saved sign-in, cookies and cache are erased and
   the app closes (SEC-018).
2. Handheld mode: Game Mode detection, full-screen controller layout, tested
   in nested Gamescope; X11 only as a fallback where there is no Wayland
   (SEC-019).

### Stage 3 pull requests _(added 2026-11-05)_

1. Microphone mute: mute our own recording stream, top-bar mute button.
2. Global hotkeys: shortcuts portal, Meta+Shift+M for mute.
3. Overlay: layer surface hosting the live page, zoom, focus, Esc and
   click-outside dismiss, Meta+Shift+X, top-bar overlay button.
4. Muted badge and invite toasts.

### Stage 2 pull requests _(added 2026-11-05)_

1. Web core and security controls: in-memory profile, site allowlist, link
   handling, permission handler, network and audio permissions, unit tests.
2. Encrypted cookies (SEC-009): Secret portal key, AES-256-GCM, never plain
   text.
3. Window and top bar: Kirigami window, system theme, 36 px top bar.
4. Shared core and settings: views over one core, translatable text,
   versioned settings file, saved window size (moved from 5, as the first
   setting).
5. Tray and window behaviour: close to tray, single instance.

## Risks and open questions

The top risk was Microsoft's page not working in QtWebEngine; the spike
answered it before any real build. _(amended 2026-11-05)_

| Risk | Likelihood | Fallback |
|---|---|---|
| Party chat blocked or broken in QtWebEngine | Low; worked in the spike _(amended 2026-11-05)_ | Switch to Electron |
| Microsoft redesigns the page | High over time | Scheduled checks, staging, small fixes |
| Flathub rejects an unofficial wrapper | Medium | GPG-signed repo on GitHub Pages |
| ~~"Crosstalk" is a registered trademark in software or telecoms~~ | Happened; renamed to Crossparty _(amended 2026-11-04)_ | |
| QtWebEngine security fixes lag Chrome | Certain | Origin allowlist limits exposure; documented accepted risk |
| Overlay on top or shortcuts portal misbehave | Low; both worked in the spike _(amended 2026-11-05)_ | KWin rule, tray control, documented KDE shortcut |
| No handheld to test on | Medium | Nested Gamescope; borrow a device before release |
| Cloud streams refuse QtWebEngine | Unknown; not yet checked _(added 2026-11-05)_ | Adjust the user agent (spike `--strip-ua`) |

### Open questions

- ~~After leaving a party, starting a new one sometimes fails with "we can't
  do this right now".~~ It happens in Firefox too, so it is Microsoft's party
  service keeping the old party open, and out of the app's reach. Workaround:
  start and leave a party on the phone (or another device), which clears it.
  To go in the README's known issues. _(answered 2026-11-05)_

- ~~Does the shortcuts portal report key release (push-to-talk)?~~ Yes (spike
  test 6). KDE sends 0 for every timestamp, so the app times presses itself.
  _(amended 2026-11-05)_
- What overlay zoom level makes Microsoft's page usable in the compact panel?
- Does the overlay's top-right position clash with your most-played games'
  HUDs?
- ~~Does QtWebEngine's own Chromium sandbox run inside Flatpak?~~ Yes (spike
  test 7). _(amended 2026-11-05)_
- ~~How does the mute hotkey mute the microphone itself?~~ By muting the
  app's own recording stream in PipeWire (see the decisions log and SEC-014).
  _(answered 2026-11-05)_

## Working agreements

How we work during the build, so new sessions pick up without re-explaining.

- **This doc is the single source of truth.** Every new session starts from
  it. _(amended 2026-11-05: it lives in the repo as docs/plan.md)_
- **Decisions log first, then code.** Any change of plan is recorded in the
  decisions log before it is built.
- **Code is written in the repo, not pasted from chat.** Either the Claude
  desktop app with the project folder connected, or Claude Code inside the
  Distrobox container. Set up in stage 0.
- **Scope freeze until v0.1.0.** New ideas go to the "Later" list in Features,
  not into the build.
- **Learning mode stays on.** Each security decision is explained when it is
  made and written into docs/security.md.

## Glossary

Plain-English meanings for the terms used in this plan.

| Term | Meaning |
|---|---|
| Allowlist | A list of the only things permitted; everything else is blocked by default |
| Attestation | A signed record proving a file was built by a specific CI run from specific code |
| Chromium | The open-source engine inside Chrome; QtWebEngine and Electron both contain it |
| CI | Continuous integration: automated checks and builds that run on every push |
| Distrobox | Runs a normal Linux container that feels like part of your desktop, so tools stay off the host |
| Flatpak | A Linux app format that runs each app in its own sandbox with listed permissions |
| Flathub | The main public store for Flatpak apps |
| Gamescope | Valve's compositor that runs Steam's Game Mode on handhelds |
| IaC | Infrastructure as code: settings written as files and applied by a tool, not clicked by hand |
| Kirigami | KDE's design kit for apps that match Plasma |
| Layer shell | A Wayland feature that lets panels and overlays sit on a layer above normal windows |
| OpenTofu | An open-source tool that applies IaC files; a fork of Terraform |
| Origin | The scheme plus domain of a web page, e.g. `https://www.xbox.com` |
| Portal | A safe door through which a sandboxed app asks the desktop for things, with your approval |
| QML | Qt's declarative language for describing interfaces |
| Sandbox | A boundary that limits what a program can reach on your system |
| Semantic versioning | Version numbers in major.minor.patch form, each part with a set meaning |
| Spike | A short throwaway build that answers one risky question before real work starts |
| State file | OpenTofu's record of what it has already set up |
| Wayland | The modern Linux display system; isolates apps from each other better than X11 |
| XWayland | Compatibility layer that runs old X11 apps inside Wayland |
| AES-256-GCM | A standard encryption method that also detects any tampering with the encrypted data _(added 2026-11-05)_ |
