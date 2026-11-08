# Crossparty

Unofficial Linux desktop app for Xbox party chat.

Crossparty puts Xbox party chat from the web (play.xbox.com) in its own KDE
window, with an overlay you can bring up over your games and a full-screen
layout for handhelds in Steam's Game Mode. Microsoft's own web app does the
party chat; Crossparty decides what it may load, what it may use and what is
kept on your machine.

**Status:** 0.2.0, from [Releases](https://github.com/KreatureofKreation/Crossparty/releases). Not on Flathub yet.

## What it does

- **Party chat in a window**, closing to the system tray so the party keeps
  going.
- **Mute hotkey**, Meta+Shift+M, which mutes Crossparty's own microphone
  stream, with a small badge in the corner while you are muted.
- **Overlay**, Meta+Shift+X, which shows the party in a panel above your
  game. It never takes the keyboard, so the game keeps focus.
- **Invite alerts**: a small notice when a party invite arrives while you are
  in a game.
- **Handheld mode**: full screen with large, controller-friendly buttons when
  started in Steam's Game Mode.
- **Sign out and wipe**: one action that signs you out and erases everything
  Crossparty saved.

## Install

Crossparty is a Flatpak. Until it is on Flathub, install the
`crossparty.flatpak` file from the latest
[release](https://github.com/KreatureofKreation/Crossparty/releases):

```bash
flatpak install --user crossparty.flatpak
```

Flatpak fetches the KDE runtime it needs from Flathub, so Flathub must be set
up (it is on Bazzite and most desktop distributions).

To check the download first, put `SHA256SUMS` from the same release next to
it and run:

```bash
sha256sum --check SHA256SUMS
```

Each release is built only by this repository's GitHub Actions, and signed
to prove it. With the [GitHub CLI](https://cli.github.com/) you can check
that too:

```bash
gh attestation verify crossparty.flatpak --repo KreatureofKreation/Crossparty
```

## First start

1. **Sign in** with your Microsoft account in the Crossparty window. Your
   sign-in is saved, encrypted. After a restart the page may show **Sign in**
   again: click it and pick your account, with no password needed. (The page
   keeps part of its state in storage Crossparty deliberately does not save.)
2. **KWallet** may ask to let Crossparty store a secret. Allow it: that is
   the key that encrypts your saved sign-in.
3. **KDE asks to approve the hotkeys** the first time. You can change them
   later in System Settings, Shortcuts.
4. **Join a party.** The first time, the page asks for the microphone; allow
   it.

## Handheld (Steam Game Mode)

Add Crossparty to Steam as a non-Steam game, then start it from Game Mode.
It opens full screen with its own bar: Mute, and Sign Out and Wipe. Use the
Steam button to switch between Crossparty and your game; the party keeps
running.

- **Sign in on the desktop first.** In Game Mode, apps share an X11 display,
  so typing your password there is less private (see Known issues).
- **Controller:** works straight away: the D-pad or left stick moves, A
  selects and B goes back, on the Xbox page and on Crossparty's bar. Press
  Menu (≡) to jump to the bar (Mute, Sign Out) and back. See
  [Controllers](#controllers).
- There is no overlay, muted badge or invite alert in Game Mode: Gamescope
  has no layer above games for them.

## Controllers

A controller works in Crossparty with no setup: the D-pad or left stick
moves, A selects and B goes back, on Crossparty's own buttons and on the
Xbox page. In handheld mode, Menu (≡) jumps between the page and
Crossparty's bar. Going back never reloads the page, so a party carries on. Crossparty only listens while it is the app in front; anything
you press in a game is never sent to the party, and Microsoft's page never
sees your controller.

- **Steam Input:** in Game Mode, Steam's usual layout for a non-Steam game
  (a gamepad) is what Crossparty expects. If you remapped Crossparty's
  layout to keys for an earlier version, set it back to a gamepad layout.
  On the desktop, if you mapped Steam's Desktop Layout D-pad to arrow keys,
  every move happens twice: undo that mapping.
- **Open the overlay from the controller.** Steam keeps the Guide button
  for itself, but a chord works over any game. In Steam, Settings,
  Controller, Guide Button Chord Layout, pick a free button (D-pad Left is
  free by default; leave A, B, X and Y alone, Steam uses them) and make it
  press the overlay's hotkey. Meta can be unreliable from a controller,
  because KDE opens the launcher on Meta alone, so add a second hotkey
  without it first: System Settings, Keyboard, Shortcuts, Crossparty, Show
  or hide the overlay, add for example Ctrl+Alt+F10, and have the chord
  press that. The same works for mute.

## Support

| Where | Status |
|---|---|
| KDE Plasma 6 on Wayland | Supported; developed and tested on Bazzite |
| Steam Game Mode (Gamescope) | Supported; tested in nested Gamescope, not yet on a real handheld |
| Other Wayland desktops | Best effort. The overlay, badge and alerts need the layer-shell protocol, which GNOME does not have |
| X11 desktop sessions | Not supported |

## Known issues

- **"We can't do this right now" when starting a party.** After leaving a
  party, starting a new one sometimes fails. It happens in a browser too: it
  is Microsoft's party service keeping the old party open. Start and leave a
  party on your phone or another device, which clears it.
- **The window does not reopen where you left it.** Wayland does not let apps
  place their own windows, so Crossparty remembers only its size. KDE can
  remember the position: System Settings, Window Management, Window Rules,
  Add New, set Window class to `io.github.KreatureofKreation.Crossparty`,
  then add the Position property and set it to Remember.
- **Controller input still reaches the game** while the overlay is open. Use
  the mouse in the overlay on the desktop.
- **Invite alerts can stop working** if Microsoft changes its page. Nothing
  else is affected, and a weekly check watches for it. There are no alerts
  for messages: the page gives no way to tell they arrived.
- **Invite alerts work only when Microsoft's page is in English.** They read
  the page's "1 invite" label, so with the page in another language no alert
  appears. Party chat itself works in any language.
- **Links that would open a new window open in your browser**, even links to
  xbox.com, so the party page is never replaced and you stay in the party.
- **Game Mode uses X11**, the only display Gamescope offers. X11 apps on the
  same display can see each other's key presses, so sign in on the desktop.
  On the desktop Crossparty never uses X11.

## Privacy

Crossparty collects nothing. There is no telemetry, analytics or crash
reporting.

- The only network traffic is to Microsoft's own sites, the same as using
  them in a browser. Every other link opens in your own browser.
- Your sign-in is saved encrypted in Crossparty's own Flatpak folder, with
  the key in your keyring. Nothing else from the page is kept after you quit.
- Settings (the window size) are one local file.
- Logs never contain cookies, tokens or full web addresses.
- Microsoft still sees everything said and done in a party.

How each of these is done, and why, is in
[docs/security.md](docs/security.md).

## Disclaimer

Crossparty is an unofficial project and is not affiliated with, endorsed by, or
sponsored by Microsoft. Xbox is a trademark of Microsoft Corporation.

Crossparty comes with no warranty (see [LICENSE](LICENSE)). It is a hobby
project, and fixes are best effort.

## Reporting a security problem

Please use GitHub's private **Report a vulnerability** button, as described
in [SECURITY.md](SECURITY.md). Do not open a public issue.

## Development

You need a Linux desktop with Podman, Distrobox and Flatpak (Bazzite has all
three). From the repository root:

```bash
dev/setup.sh
```

```bash
distrobox enter crossparty-dev
```

Then inside the container:

```bash
dev/lint.sh
```

```bash
dev/flatpak-build.sh
```

See [CONTRIBUTING.md](CONTRIBUTING.md) for the code style and workflow,
[docs/security.md](docs/security.md) for why things are the way they are, and
[docs/plan.md](docs/plan.md) for the project plan.

## Licence

MIT. Qt and QtWebEngine are LGPL. They are supplied unmodified by the Flatpak
runtime and dynamically linked.
