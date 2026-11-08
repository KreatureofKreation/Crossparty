# Changelog

Every release is listed here in plain English. Versions follow semantic
versioning: patch for fixes, minor for features, major for breaking changes.

## 0.2.0 - 2026-11-08

- **Controllers work with no setup:** the D-pad or left stick moves, A
  selects and B goes back, on Crossparty's buttons and on the Xbox page,
  only while Crossparty is the app in front. Microsoft's page never sees
  the controller.
- **Menu (≡) reaches the handheld bar:** in handheld mode it jumps
  between the Xbox page and Mute and Sign Out, and back.
- **Keys go straight to the page:** no need to tab into it first.
- **Back never drops the party:** going back (B, or a mouse's Back button)
  only steps through the Xbox app's own pages, and never reloads it.

## 0.1.0 - 2026-11-07

The first release.

- **Party chat window:** Xbox party chat from play.xbox.com in a KDE window
  with its own title bar, following the system theme. Closing it keeps the
  party running in the tray; only one copy runs at a time.
- **Sign-in kept, encrypted:** sign-in cookies are saved with AES-256-GCM,
  with the key in your keyring. Nothing else from the page is written to
  disk.
- **Only Microsoft's sites inside the app:** every other link opens in your
  browser, and the page gets the microphone and nothing more.
- **Mute:** Meta+Shift+M or the top-bar button mutes Crossparty's own
  microphone stream, with a corner badge while muted.
- **Overlay:** Meta+Shift+X shows the party in a panel above games, without
  taking the keyboard from them.
- **Invite alerts:** a small corner notice when a party invite arrives while
  the party is out of sight.
- **Handheld mode:** full screen with a controller-friendly bar in Steam's
  Game Mode.
- **Sign out and wipe:** one confirmed action erases the saved sign-in,
  cookies and cache, then closes the app.
- **Project:** dev container, formatters, linters, git hooks, CI with
  CodeQL and Flathub's linter, and the GitHub settings as OpenTofu code.
- Renamed from Crosstalk to Crossparty before any release. A trademark
  search found a live US registration for "Crosstalk" covering software for
  calls between users. The app ID is `io.github.KreatureofKreation.Crossparty`.
