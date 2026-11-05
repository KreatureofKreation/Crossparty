# Stage 1 spike results

The spike is a throwaway build that answers whether Qt and QtWebEngine can
carry the app. The pass/fail rules were decided before it ran, in the project
plan's Testing plan. Every test runs inside a Flatpak build, not on the host.

**Verdict:** Qt. Seven of eight tests pass; test 8's failure has a workaround (SEC-009). Test 1 still needs a check once a cloud stream starts.

| Test | Pass if | If it fails | Result | Notes |
|---|---|---|---|---|
| xbox.com/play loads in QtWebEngine | Page loads, with no unsupported-browser block | Adjust the user agent; still blocked after two sessions → Electron | Provisional pass | Cloud gaming page loads with the stock user agent, no banner. Recheck when a stream starts (needs sign-in). |
| Sign-in persists | Still signed in after restarting the app | Fix storage; unresolved after two sessions → Electron | Pass (disk mode) | Still signed in after quitting and restarting. |
| Party voice | Two-way audio with the other account for 10 minutes | Unresolved after two sessions → Electron | Pass | Two-way party voice for 10+ minutes with a second account. Connected through Microsoft's voice server (DTLS encrypted), Opus in both directions. Slight latency, normal for party chat. |
| Overlay as a layer surface | Sits above a borderless game, takes focus, hides on Esc | Fall back to a KWin window rule, documented in the README | Pass | Shows top-right above a borderless game, takes typing straight away, hides on Esc and on the hotkey. Over an exclusive full-screen game, the game drops back to windowed when the overlay takes the keyboard. |
| Global shortcuts portal | KDE approval prompt appears; hotkeys fire in-game | Tray-only control plus a documented KDE custom shortcut | Pass | KDE's dialog appears with the preferred keys (`LOGO+SHIFT+x` form) and both hotkeys fire. Later binds return the saved keys with no dialog. |
| Key release events | Portal reports key release | Push-to-talk stays cut | Pass | Deactivated arrives on the real key release: holds of 4.8 s and 2.7 s measured by the app's own clock. KDE sends 0 for every timestamp, so the app must time presses itself. |
| Chromium sandbox inside Flatpak | QtWebEngine's own sandbox runs alongside Flatpak's | Rely on Flatpak's sandbox alone; update docs/security.md | Pass | `chrome://sandbox`: layer 1 Flatpak, PID and network namespaces, seccomp-BPF with TSYNC, "adequately sandboxed". The Yama ptrace rows say No; Flatpak's own seccomp filter blocks ptrace. |
| Keyring inside Flatpak | Cookies saved encrypted through Secret Service | Fix before stage 2; never fall back to plain-text storage | **Fail** (as predicted) | All 44 cookies are stored as plain text, including the Microsoft sign-in cookies (`MSPAuth`, `WLSSC`, `__Host-MSAAUTHP`) and the Xbox Live token cookies (`XBXXtk…`). The profile folder is owner-only (700, Cookies file 600). Workaround found: in-memory profile plus encrypted cookies (SEC-009). |

## Sessions

<!-- One entry per spike session: date, what was tried, what happened. -->

### 2026-11-04: research before the first run

**Test 8 is expected to fail with stock QtWebEngine.** QtWebEngine 6.11 never
encrypts its cookie database. `src/core/profile_io_data_qt.cpp` sets
`enable_encrypted_cookies = false` for every profile, with the comment that
os_crypt "currently has issues for us on Linux". The same line is in 6.8 and in
Qt's development branch, so no update will change it soon. The spike measures
what is really on disk (`spike/inspect-storage.py`). It also tries a
workaround: an in-memory profile, with the app saving cookies itself
(`--cookie-mode=memory-jar`). If cookies alone keep you signed in, a real build
could encrypt that file with a Secret Service key.

Even Chrome and Electron only encrypt cookies. Local Storage and IndexedDB,
where web sign-in libraries often keep tokens, are plain files in every
Chromium-based app.

**Test 7 looks promising.** The Flathub QtWebEngine base app patches Chromium
for sandboxing inside Flatpak
(`chromium-flatpak-add-initial-sandbox-support.patch`). The spike checks the
result on `chrome://sandbox`.

**LayerShellQt is not in the KDE runtime.** The spike builds layer-shell-qt
v6.8.2 as an extra module. The real app will do the same.

**New question, not in the table:** the mute hotkey has to mute the
*microphone*. QtWebEngine can only mute a page's audio *output*, and the plan
rules out scripting Microsoft's page. Muting our own capture stream through
PulseAudio/PipeWire is the likely route. It needs checking before stage 3.

### 2026-11-04: first run, shortcuts

KDE stores portal shortcuts in `~/.config/kglobalshortcutsrc` under the app
ID, and from then on BindShortcuts returns them without a dialog. The first
build sent `SUPER+…` triggers, which KDE did not recognise, so the user
approved two empty shortcuts. Removing them in System Settings left the
`none` entries in place, and no dialog came back. They only went away through
KGlobalAccel's `unregister` D-Bus call.

For the real app: get the trigger names right the first time (`LOGO`, not
`SUPER`), and offer a way to change keys later. Version 2 of the portal has
`ConfigureShortcuts` for that.

Key release (test 6) works: KDE sends Deactivated when the key is let go, not
straight after the press. But KDE sends 0 for every timestamp, so the app
times presses with its own clock. Each run also has to open its own portal
session (CreateSession, then BindShortcuts) before any hotkey reaches it. KDE
keeps the saved keys, so no dialog appears after the first one.

The overlay passes over a borderless game. Over an exclusive full-screen game,
the game drops back to windowed when the overlay takes the keyboard. Worth
trying on-demand keyboard focus in stage 3.

**New requirement, raised by the user, not in the plan yet:** the hotkeys give
no feedback. Pressing the mute key in a game shows nothing, so the user can't
tell whether it worked. The real app should tell them, for example "Microphone
muted" and "Microphone on". The likely route is the XDG notification portal
(`org.freedesktop.portal.Notification`), which works from inside Flatpak
without extra permissions. Showing it briefly in the overlay is the fallback
for games in exclusive full screen. The scope is frozen until v0.1.0, so this
needs a decisions-log entry in the plan before stage 3.

### 2026-11-04: sign-in and storage

Signed in with the spare account in disk mode and chose "Stay signed in".
The sign-in flow (xbox.com → login.live.com → back) ran inside the spike with
no browser errors and no pop-up windows.

`spike/inspect-storage.py` after quitting: 44 cookies, all plain text, none
encrypted. They include the long-lived Microsoft account cookies and the Xbox
Live tokens. Local Storage (47 KiB), IndexedDB (1 MiB) and Session Storage
(303 KiB) hold more data, also unencrypted, as in every Chromium-based app.
Only the user's own account can read the files: the folder is 700, and the
Cookies file is 600.

### 2026-11-05: memory-jar workaround

In-memory profile, with the spike saving cookies to its own file
(`--cookie-mode=memory-jar`).

- First try: after a restart, Microsoft remembered the account but asked to
  verify again. A spike bug caused it. Host-only cookies were saved with a
  `domain=` attribute, so Chromium rejected `__Host-MSAAUTHP` on restore.
  Fixed by clearing the domain for host-only cookies.
- Second try: 41 cookies restored, 6 of them host-only. xbox.com showed
  signed out at start. Clicking Sign in, then the account, signed straight in,
  with no password or verification.

So the cookies carry the Microsoft session. xbox.com keeps its own signed-in
state in Local Storage, which an in-memory profile drops on quit.
QtWebEngine has no API to save Local Storage from an in-memory profile without
scripting Microsoft's page, and the plan rules that out.

The plain-text cookie file was deleted after the test.

**Decision (2026-11-05):** encrypted cookies, recorded as SEC-009 in
`docs/security.md`. The spike's plain-text on-disk profile for the spare
account was deleted.

### 2026-11-05: party voice

Spare account in the spike (in-memory mode), partner on another account. After
restoring the saved cookies, signing in took only picking the account: no
password, no code. The spike granted microphone access to `https://www.xbox.com`
automatically. Voice worked both ways for over 10 minutes, with slight latency.
`chrome://webrtc-internals`: one peer connection to Microsoft's voice server,
16 incoming audio slots (one per possible party member) with one in use, one
outgoing Opus stream with in-band FEC and DTX.

Opening `chrome://webrtc-internals` in the main view left the Xbox page and
ended the party. The spike now opens it in its own window.
