# Stage 1 spike results

The spike is a throwaway build that answers whether Qt and QtWebEngine can
carry the app. The pass/fail rules were decided before it ran, in the project
plan's Testing plan. Every test runs inside a Flatpak build, not on the host.

**Verdict:** _not run yet_ (Qt, or switch to Electron)

| Test | Pass if | If it fails | Result | Notes |
|---|---|---|---|---|
| xbox.com/play loads in QtWebEngine | Page loads, with no unsupported-browser block | Adjust the user agent; still blocked after two sessions → Electron | | |
| Sign-in persists | Still signed in after restarting the app | Fix storage; unresolved after two sessions → Electron | | |
| Party voice | Two-way audio with the other account for 10 minutes | Unresolved after two sessions → Electron | | |
| Overlay as a layer surface | Sits above a borderless game, takes focus, hides on Esc | Fall back to a KWin window rule, documented in the README | | |
| Global shortcuts portal | KDE approval prompt appears; hotkeys fire in-game | Tray-only control plus a documented KDE custom shortcut | | |
| Key release events | Portal reports key release | Push-to-talk stays cut | | |
| Chromium sandbox inside Flatpak | QtWebEngine's own sandbox runs alongside Flatpak's | Rely on Flatpak's sandbox alone; update docs/security.md | | |
| Keyring inside Flatpak | Cookies saved encrypted through Secret Service | Fix before stage 2; never fall back to plain-text storage | | |

## Sessions

<!-- One entry per spike session: date, what was tried, what happened. -->
