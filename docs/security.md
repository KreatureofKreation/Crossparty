# Security notes

This file explains every security decision in Crossparty, in plain English.
Code that makes one of these decisions links here with a comment such as
`docs/security.md#sec-001`.

The full threat model (assets, threats and controls) is written up in
stage 5. Until then, the Security and threat model section of the project
plan is the reference.

## Decision log

### SEC-001: Development tools live in a container, not on the host

**Decision:** Compilers, Qt dev packages, linters and OpenTofu are installed
in a Distrobox container built from `dev/Containerfile`. CI lints with the same
image.

**Why:** Bazzite's host is immutable, and every package layered onto it is
more to trust and to keep patched. A container can be deleted and rebuilt
from the repo at any time. Because CI uses the same definition, a check that
passes locally passes in CI too.

**Limit:** Distrobox is for convenience, not a security boundary. It shares
your home folder and runs as your user, so a malicious build tool inside it
could still read your files. The real defence is choosing what goes in:
Fedora's signed packages only.

### SEC-002: Third-party code in CI is pinned by hash

**Decision:** Every GitHub Action is pinned to a full commit hash, and every
container image to a digest. The tag stays alongside as a comment for
readability. The one outside pre-commit hook repo is pinned the same way.

**Why:** A tag like `v7` can be moved to point at different code. If an
action's maintainer account were taken over, a moved tag would run the
attacker's code in our CI. A commit hash cannot be moved. Dependabot proposes
hash updates as pull requests to `staging`, so updates are still reviewed.

### SEC-003: CI gets a read-only token

**Decision:** The workflow sets `permissions: contents: read`, and checkout
uses `persist-credentials: false`.

**Why:** If a build step or dependency were compromised, the token it could
steal can only read the repo. It cannot push code or change releases.
Not leaving the token in `.git/config` means later steps cannot pick it up
by accident.

### SEC-004: Secrets are caught before they are committed

**Decision:** Every commit runs `gitleaks` and `detect-private-key` on the
staged changes. CI scans the whole git history with `gitleaks`.

**Why:** A secret pushed to GitHub has to be treated as leaked, even after it
is deleted, because the history keeps it. Catching it before the commit is
far cheaper than rotating it afterwards. GitHub's own secret scanning with
push protection is added in stage 5 as a second layer.

### SEC-005: Flatpak permissions are added only when a feature needs them

**Decision:** The manifest starts with only `--socket=wayland` and
`--device=dri`. Each later permission (network, audio, tray, Secret Service)
is added in the stage that needs it, with a comment giving the reason.

**Why:** Deny by default. A permission that is not granted cannot be abused,
and a short list is easy to review. There is deliberately no X11 socket: X11
lets any client read other clients' keystrokes, including games running on
XWayland.

**Testing:** The unit tests run headless (`QT_QPA_PLATFORM=offscreen`), so
even the test sandbox needs no display socket. The CI Flatpak action's
`run-tests` input is left off because it would add an X11 socket and network
access to the test sandbox.

### SEC-006: The app identifies itself by its app ID

**Decision:** `main.cpp` sets the desktop file name to
`io.github.KreatureofKreation.Crossparty`, the same as the Flatpak ID.

**Why:** On Wayland, the desktop file name becomes the app ID that KDE's
portals use. When you approve global shortcuts or notifications, the approval
is stored against that ID. Keeping one ID everywhere means approvals cannot
be confused with another app's, and it is why the ID must never change.

### SEC-007: Fail safe on startup

**Decision:** If the interface fails to load, the app exits with an error.

**Why:** An app that keeps running with no window has nothing showing its
state. Once the app manages a microphone, that would mean audio you cannot
see or stop. Failing closed is the rule throughout (plan: Clean, readable
code).

### SEC-008: Pushing to GitHub uses a dedicated SSH key

**Decision:** Git signs in to GitHub with an Ed25519 SSH key made only for
GitHub (`~/.ssh/id_ed25519_github`), protected by a passphrase. `~/.ssh/config`
sets `IdentitiesOnly yes` for `github.com`, so no other key is ever offered.

**Why:** If this key leaks, revoking it affects nothing else you use. A
passphrase means a copied key file is useless on its own. SSH, unlike an HTTPS
token, involves no long-lived secret typed into a terminal or stored in git's
config. GitHub's host fingerprint was checked against the one GitHub publishes
before the first connection.

**Follow-up:** Move to a key that lives on the hardware security key
(`ed25519-sk`) when one is set up, then delete this key from GitHub. With that
key, the secret cannot be copied at all, and each push needs a physical touch.

### SEC-009: Sign-in cookies are stored encrypted, and nothing else is kept

**Decision:** The app uses an in-memory (off-the-record) QtWebEngine profile,
so Chromium writes no cookies, Local Storage or IndexedDB to disk. The app
saves only the cookies itself, encrypted with a key held in the user's keyring
(KWallet, reached through the XDG Secret portal), and restores them at start.

**Why:** The spike (test 8) showed that QtWebEngine always stores its cookie
database as plain text, including Microsoft's sign-in cookies and the Xbox Live
tokens. Anyone who copies that file can act as the account. The plan's rule is
never to fall back to plain-text storage. The spike also showed that the saved
cookies alone restore the Microsoft session, with no password needed.

**Cost:** xbox.com keeps its own signed-in state in Local Storage, which is
dropped on quit. Each launch shows the user signed out until they click Sign in
and pick their account. Stage 2 will try to remove that click by opening
xbox.com's sign-in address at start. That is navigation only; it does not
script Microsoft's page.

**How it works:**

- The key comes from the XDG Secret portal, which KDE backs with KWallet. The
  portal gives each Flatpak app its own secret, tied to the app ID, and needs
  no Flatpak permission. It writes the secret into a pipe the app hands it,
  so the secret never travels over D-Bus. The app turns it into a 32-byte key
  for this one purpose with HKDF-SHA256.
- Cookies are encrypted with AES-256-GCM (OpenSSL), with a new random nonce on
  every save. GCM also detects any change to the file: a changed, cut-short or
  foreign file fails to decrypt, and is then deleted, so the user starts
  signed out.
- The file is `cookies.vault` in the app's own data folder, owner-only (600),
  written atomically so a crash never leaves half a file. Expired cookies are
  left out. Session cookies are kept, because the spike showed sign-in needs
  them.
- Fail safe: if the portal refuses, KWallet stays locked for 60 seconds, or
  the key cannot be made, nothing is saved. The app still works; the user
  signs in again next time.
- Host-only cookies are restored without a domain. With one, Chromium refuses
  Microsoft's `__Host-` sign-in cookie (found in the spike).
- Tests: `tests/CookieCipherTest.cpp` (round trips, fresh nonces, wrong key,
  every changed byte) and `tests/CookieFormatTest.cpp`.

**Limit:** anything that can run code as the user and reach KWallet while it
is unlocked can still ask for the secret, the same as for any keyring. This
protects against the cookie file being copied or read on its own, which is
what the plain-text database allowed.

**Rejected:** keeping the normal on-disk profile (plain-text tokens readable by
any unsandboxed program running as the user), and switching to Electron (it
encrypts cookies but still keeps Local Storage in plain text, and it cannot
draw the overlay as a layer surface).

### SEC-010: Only Microsoft's sites load inside the app

**Decision:** The page may only show addresses on an exact list of hosts:
`www.xbox.com`, `login.live.com`, `login.microsoftonline.com` and
`account.live.com`, over https on the standard port, with no user name in the
address. Any other https link opens in the user's default browser, through the
OpenURI portal. Everything else (plain http, `file:`, `data:`, `javascript:`,
other apps' link schemes) is blocked. Requests to open a new window follow the
same rules, and downloads are refused.

**Why:** The main attack path is a link in party text chat. Inside the app, a
malicious page would sit next to the Microsoft session and the microphone
permission. In the user's own browser, it is just another web page. Exact
host names, not patterns, mean a look-alike such as
`www.xbox.com.attacker.example` can never match. A user name in an address
(`https://www.xbox.com@attacker.example`) is refused because it hides the real
host from anyone reading the link. Unknown link schemes are blocked rather
than handed to the desktop, because they could open local files or start
other programs.

**Frames:** Frames embedded in an allowed page may load from other hosts,
because Microsoft's pages use them (for example for sign-in checks) and a
frame cannot be moved to the browser on its own. A frame can never replace the
main page, and it gets no permissions unless its own origin is allowed
(SEC-011).

**Logs:** Decisions are logged with the scheme and host only, never the full
address, which can carry personal details or sign-in tokens.

**Tests:** `tests/SiteAllowlistTest.cpp` and `tests/NavigationPolicyTest.cpp`
cover the tricks above. The host list came from where the stage 1 spike went
while signing in and joining a party (`docs/spike.md`). A host is added only
after seeing the page need it.

### SEC-011: Browser permissions are denied by default

**Decision:** Only two permissions are ever granted: the microphone and
notifications, and only to pages on the allowlist (SEC-010). Every other
request (camera, screen sharing, location, clipboard, mouse lock, local fonts,
and any type added in a future QtWebEngine) is denied. The web profile is
in-memory, so no decision is remembered across restarts.

**Why:** Party chat needs the microphone, and invite alerts arrive as browser
notifications. Playing sound needs no permission. Anything more would widen
what a compromised or malicious page could do with no benefit to the user.
Listing what is allowed, rather than what is refused, means a new kind of
permission is refused automatically.

**Tests:** `tests/PermissionPolicyTest.cpp`.

### SEC-012: Network and audio access for the Flatpak

**Decision:** The manifest adds `--share=network` and `--socket=pulseaudio`,
and builds on Flathub's QtWebEngine base app.

**Why:** The app cannot reach Microsoft's servers without the network, and
party chat needs the microphone and speakers. Flatpak has no narrower option:
network access is all or nothing, and the PulseAudio socket covers both
playback and recording. The narrowing happens inside the app instead: the
allowlist (SEC-010) decides which sites load, and the permission policy
(SEC-011) decides which of them may use the microphone. The QtWebEngine base
app is Flathub's own build of QtWebEngine, with Chromium patched so its
sandbox runs inside Flatpak (spike test 7).

### SEC-013: Tray icon over D-Bus, and one copy at a time

**Decision:** The manifest adds `--talk-name=org.kde.StatusNotifierWatcher`,
so the app can show its icon in KDE's system tray. The app also claims its own
app ID on the session bus and answers `org.freedesktop.Application.Activate`;
a second copy finds the name taken, asks the first to show its window, and
exits.

**Why:** Closing the window keeps the party running in the tray, so the tray
icon is the way back to it. The permission lets the app talk to the tray's
watcher only, not to any other program on the bus. Owning its own app ID
needs no permission in Flatpak. One copy at a time means there is never a
second party connection, and never a second process holding the same
microphone and session.

**Fail safe:** with no tray, closing the window quits the app, so it is never
left running with no way to reach it. Only the first copy loads Microsoft's
page; a second copy exits before creating any web content.

### SEC-014: The mute silences only the app's own microphone stream

**Decision:** The mute works on the recording stream itself, through the
PulseAudio API that PipeWire provides. The app picks out streams whose
`pipewire.access.portal.app_id` is its own app ID, and mutes or unmutes only
those. A new stream (the page reopening the microphone) gets the user's
choice; a change made elsewhere, such as in Plasma's volume applet, is
followed.

**Why:** QtWebEngine can only mute what a page plays, and the plan rules out
scripting Microsoft's page to press its own mute button. Muting the stream
means Microsoft's page receives silence; nothing about the page changes.
PipeWire sets the app ID property itself, from the Flatpak sandbox's records,
so an app cannot claim another app's stream. Inside the sandbox, the app can
only see its own audio streams anyway (checked on 2026-11-05: other apps'
streams were not listed).

**Permissions:** none new. The PulseAudio socket is already granted for party
chat (SEC-012).

**Fail safe:** outside Flatpak there is no trustworthy app ID, so the mute
button is disabled rather than guessing. If the sound server cannot be
reached, the button is disabled too.

**Tests:** `tests/MicrophoneStreamsTest.cpp` (which streams count as ours,
and what happens to new and changed streams).

## Accepted risks

### RISK-001: QtWebEngine gets Chromium security fixes later than Chrome

QtWebEngine is patched through the KDE runtime, which lags Chrome's releases.
Exposure is limited because only Microsoft's own origins ever load in the app,
and every other link opens in your default browser. This risk is reviewed at
each release.
