# Security notes

This file explains every security decision in Crossparty, in plain English.
Code that makes one of these decisions links here with a comment such as
`docs/security.md#sec-001`.

The threat model below says what is protected, from whom, and how. Each
control links to the decision that explains it, and what the controls cannot
stop is listed plainly under Accepted risks.

## Threat model

_Written in stage 5, 2026-11-06. Reviewed at every release, and whenever a
Flatpak permission, an allowed host or a new SEC entry is added._

### What Crossparty is

A Flatpak app that shows Microsoft's Xbox web app (play.xbox.com) in a KDE
window, an overlay above games and a handheld layout. Microsoft's page does
the party chat; Crossparty decides what that page may load, what it may use,
and what is kept on disk. It runs no server, has no account of its own and
sends nothing anywhere except the page's own traffic to Microsoft.

### Trust boundaries

Data crossing these lines is checked; nothing inside a line is trusted to
check itself.

1. **Party chat to the page.** Other players' text and links arrive inside
   Microsoft's page. Microsoft is trusted to run its own service; what other
   players send is not.
2. **The page to the app.** The page can ask to navigate, open windows, use
   permissions and write console messages. The C++ core decides every one
   (SEC-010, SEC-011, SEC-017).
3. **The app to the desktop.** Flatpak's sandbox, with Chromium's own sandbox
   inside it (SEC-012). The app reaches the desktop only through the listed
   Flatpak permissions and portals (SEC-005).
4. **The app's files to other programs.** Anything on disk can be read by
   other programs running as you, so sign-in data is only ever on disk
   encrypted (SEC-009).
5. **Source to release.** Code, dependencies, CI and the published build
   (SEC-001 to SEC-004, SEC-020).

### Assets

| Asset | Why it matters |
|---|---|
| Microsoft sign-in (cookies and tokens) | Whoever holds them is signed in as you, to Xbox and possibly more of your Microsoft account |
| Microphone | Live audio from your room |
| Keystrokes | Passwords and chat typed while Crossparty or a game has focus |
| The release download | A swapped build runs on every user's machine |
| The GitHub account, its keys and tokens | Control of the code, settings and future releases |

### Who might attack

| Attacker | What they can do to start with |
|---|---|
| Another player in a party | Send text and links that appear in the page |
| A malicious website | Run its own code, if a link to it ever loads |
| Someone on the network | See and change unencrypted traffic |
| Someone with the device | Use the logged-in desktop, or copy files from a lost or shared device |
| Other software running as you | Read your files and talk to your session's services |
| A supply chain attacker | Tamper with a dependency, a CI action or a build tool |
| Someone after the developer's accounts | Phish or steal a GitHub login, key or token |

Microsoft is not in this list: the page is Microsoft's own, and it already
sees everything said and done in a party. Crossparty cannot hide anything from
Microsoft and does not try.

### Threats and controls

| Threat | Controls | Left over |
|---|---|---|
| A link in party chat leads to a malicious page next to your session and microphone | Exact host allowlist, https only, every other link to your browser, other schemes blocked, downloads refused (SEC-010) | Frames inside Microsoft's pages may load other hosts, with no permissions (SEC-010) |
| A page uses the camera, screen, location or other browser powers | Only the microphone and notifications, only for allowed hosts; any new permission type refused (SEC-011) | None beyond what Microsoft's own page does with the microphone |
| The page breaks out of the browser | Chromium's sandbox inside Flatpak's (SEC-012); no native objects exposed to the page; the invite watcher runs in an isolated world and only reports a number (SEC-017) | A Chromium bug fixed in Chrome but not yet in QtWebEngine (RISK-001) |
| The sign-in is copied from disk | In-memory web profile; cookies saved only with AES-256-GCM, key from KWallet via the Secret portal; tampered files deleted (SEC-009) | Software running as you while KWallet is unlocked can ask for the key (RISK-002) |
| A lost or shared device keeps the sign-in | Sign out and wipe erases the saved sign-in, cookies and cache, then quits (SEC-018) | The KWallet secret stays, protecting nothing on its own (SEC-018) |
| The microphone is live without you knowing | Mute silences Crossparty's own recording stream; the muted badge and top bar always show the state; one copy at a time; the app quits rather than run with no window (SEC-007, SEC-013, SEC-014) | The page decides when it records while unmuted, as in any browser |
| Keystrokes are read by another app | Wayland only on the desktop; the overlay and notices never take the keyboard (SEC-015, SEC-016) | X11 in Steam's Game Mode, shared with games (RISK-003, SEC-019) |
| Controller presses in a game reach the party, or Microsoft sees them | Controllers are read only while Crossparty is the active app, presses made elsewhere are thrown away, and only arrow keys, Enter and Escape are sent; Chromium has no /run/udev, so the page never sees a controller (SEC-024) | The permission itself could read keyboards and mice; only Crossparty's code keeps to controllers and to focus (RISK-004) |
| Page text in notices is used to trick you | Shown as plain text only, never links or rich text; invite toasts use fixed text (SEC-016, SEC-017) | None |
| Traffic is read or changed on the network | Allowed hosts are https only (SEC-010); certificate checks are Chromium's own | None beyond Chromium's |
| A tampered dependency or CI step | Dev tools from Fedora's signed packages in a container (SEC-001); actions and images pinned by hash, read-only CI token (SEC-002, SEC-003); pinned Flatpak sources and an offline build; CodeQL (SEC-020); Dependabot updates reviewed on staging | A compromise inside an upstream release that passes review |
| A secret is committed | gitleaks and private-key checks on every commit, and on the whole history in CI (SEC-004); GitHub secret scanning with push protection (SEC-021) | Caught only for formats the scanners know |
| The developer's GitHub account is taken over | Dedicated passphrase-protected SSH key (SEC-008); 2FA; rulesets so nothing reaches main or staging without passing checks, and release tags cannot move; admin token made only for each apply (SEC-021) | Hardware-key 2FA still to do (stage 0's open item) |
| A tampered release reaches users | Built only in CI from a tagged commit on main, from scratch; SHA-256 checksum and signed build provenance; tags cannot move (SEC-021, SEC-023) | Users who skip the checks; Flathub's signed repository once published there |

### Out of scope

- Anything Microsoft does with what you say, play or type on its service.
- A device already fully compromised (root access, or a keylogger in the
  desktop session).
- Cheating or anti-cheat systems: Crossparty never reads or changes games.
- Other Microsoft services reached by your browser through links Crossparty
  hands to it.

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
and a short list is easy to review. There is deliberately no X11 socket on
the desktop: X11 lets any client read other clients' keystrokes, including
games running on XWayland. Steam's Game Mode, which offers nothing but X11, is
the one exception (SEC-019) _(amended 2026-11-06)_.

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
`www.xbox.com`, `play.xbox.com`, `login.live.com`,
`login.microsoftonline.com` and `account.live.com`, over https on the standard port, with no user name in the
address. Any other https link opens in the user's default browser, through the
OpenURI portal. Everything else (plain http, `file:`, `data:`, `javascript:`,
other apps' link schemes) is blocked. Requests to open a new window never load
in the app, even for the hosts above: the one page holds the party, and
loading anything in its place would drop the user from it. Any https address
opens in the browser and everything else is blocked. Downloads are refused.

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
after seeing the page need it. `play.xbox.com` was added in stage 3: after
picking an account, sign-in returned there rather than to `www.xbox.com/play`,
so the app sent the user out to their browser and the sign-in was lost. It
serves the same Xbox app, run by Microsoft.

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
reached, the button is disabled too, and the hotkey does nothing: the app
never shows "muted" when it cannot mute. If the connection is lost while
muted, the app shows the microphone as on (the overlay says "Mute
unavailable") and tries to reconnect every 3 seconds. Once back, it follows
each stream's real state rather than applying an old choice, so a stream
that stayed muted is never unmuted.

**Tests:** `tests/MicrophoneStreamsTest.cpp` (which streams count as ours,
and what happens to new and changed streams).

### SEC-015: The overlay is a layer surface that never takes the keyboard

**Decision:** The overlay is a Wayland layer-shell surface on the overlay
layer, built with LayerShellQt (pinned to v6.8.2 by tag and commit, built
from source in the manifest). It never takes keyboard focus: it is used with
the mouse, and opened and closed with the global hotkey. The one live party
page moves into it while it is open.

**Why:** Layer shell is how Wayland lets panels sit above other windows,
including full-screen games, without the X11 tricks that would let apps read
each other's keystrokes. A first version took the keyboard when opened; both
games tried (Baldur's Gate 3, Dragon's Dogma 2) minimised or left full screen
as soon as they lost it. With no keyboard, the game keeps focus and the
overlay can never trap the keyboard. Moving the same page, rather than
opening a second one, keeps one party connection and one microphone stream.

**Cost:** no typing in the overlay (party text chat stays in window mode),
and no Esc or click-outside to close it.

**Supply chain:** LayerShellQt is KDE's own library. The manifest pins the
exact commit, so a moved tag cannot change what is built.

### SEC-016: Notices above games show the page's text as plain text, and let clicks through

**Decision:** The muted badge and the invite and message toasts live in one
small layer surface in the top-right corner. It never takes the keyboard and
is transparent to input, so clicks go to the game underneath. Toast text comes
from the page's own browser notifications, only from allowlisted origins, and
is always shown as plain text. The text is never logged.

_(Amended 2026-11-06.)_ xbox.com turned out to send no browser notifications,
so today the only toast is the invite toast from SEC-017, whose text is fixed
by the app. The plain-text rule stays, for any notification Microsoft adds
later.

**Why:** A notification's title and message are written by whoever sent the
invite or message, through Microsoft's page. Qt's text items guess whether
text is rich text (HTML) unless told otherwise, so a crafted message could
otherwise show images, styling or links that look like part of the app.
Plain text removes that, and links can never be followed from a toast. An
input-transparent surface can never catch a click meant for the game.

### SEC-017: A read-only script watches the invite count

**Decision:** One small script runs in the page to notice party invites. It
runs in Chromium's isolated world (`ApplicationWorld`), on the main page only,
never in frames. It reads the text of one button (`nav-guide`, which shows "1
invite" in the page's narrow layout) and reports only a number, as a console
message, and only once the count has held steady for 5 seconds, so the
label appearing late or flickering never looks like a new invite. It never
changes the page, clicks anything, or reads names or
messages. The app acts on a report only when the page is on the allowlist
(SEC-010) and the report carries this run's token: 128 random bits made at
startup and written only into the script's source. The page and its frames
can write console messages too, but they cannot read an isolated-world
script, so they cannot fake or hide a report. A report can do one thing:
show a toast with fixed text, "Party invite". To make the count visible while the user is in a game, the page is
laid out at the overlay's narrow width whenever the window is not on screen.
Chromium only lays out a page in a window that is being drawn, and KDE stops
drawing minimised windows, so the page then moves to a one-pixel, transparent,
click-through layer surface on the desktop's background layer, under every
other window. Nothing of the page can be seen there, and it takes no input. The page's other console messages are no longer printed by
default; they are logged under `crossparty.page` at debug level.

**Why:** xbox.com has no browser notifications and no pop-ups of its own in a
browser, so there was no other way to tell the user about an invite while
they play (checked in the page's code and in the running app). "No
scripting" was the rule because scripts that change Microsoft's page break
whenever Microsoft updates it, and a script in the page's own world could be
tampered with by, or tamper with, the page. Isolation means the page's code
cannot see or change the script, and the script cannot reach the page's code
or the signed-in session's JavaScript objects. Reading one label and sending
one number is the smallest thing that works. Because the toast text is fixed,
nothing a stranger writes can appear above a game.

**Limits, accepted:**
- If Microsoft renames the button or changes its wording, the reports stop
  and so do the toasts; nothing else breaks. A weekly check watches for this
  (`docs/plan.md`, Scheduled checks). The wording match is English only.
- A toast arrives about 5 seconds after the invite.

**Tests:** `tests/InviteWatchTest.cpp` checks that only a rising count raises
a toast, that a count seen again after the wide layout does not, that every
other message is ignored (including reports without this run's token), that
each run has its own token, and that the script is isolated and main-page
only.

### SEC-018: Sign out and wipe leaves nothing behind

**Decision:** Sign out and wipe, in the top bar's options menu and the tray
menu, asks once (Cancel is the default button), then:

1. Deletes the encrypted cookie file, forgets its key, and stops saving, so
   nothing can be written again in this run.
2. Deletes every cookie in the web profile and the HTTP cache.
3. Closes the app.

**Fail safe:** if the file cannot be deleted, the app does not close or claim
success. Steps 1 and 2 still happen, so the user is signed out for this run,
and a dialog names the file to delete by hand; otherwise it would sign them
back in next time.

**Why:** On a shared or lost device, one action must remove access to the
Microsoft account from Crossparty. The web profile is in-memory (SEC-009), so
local storage, databases, service workers and visited links exist only in the
running process, but QtWebEngine has no call to clear them; ending the process
is the one way to be sure they are gone. The vault is wiped first so that the cookie
deletions in step 2 can never be saved as a new file. The secret stored in
KWallet through the Secret portal is not removed (the portal has no call for
it); without the file it protects nothing, and a new file made after the next
sign-in uses it again. Settings (the window size) are not session data and
are kept.

**Tests:** manual (docs/plan.md, Manual checklist: "Sign out and wipe leaves
no session data"). The cookie store cannot run inside the build sandbox, so
there is no unit test.

### SEC-019: X11 only in Game Mode, where there is no Wayland

**Decision:** The Flatpak asks for `fallback-x11` as well as `wayland`.
Flatpak then gives X11 access only when no Wayland display is running. On the
Plasma desktop, Wayland is always running, so Crossparty stays Wayland-only
there, exactly as before.

**Why:** Gamescope, the compositor behind Steam's Game Mode, offers the apps
it runs only X11 (its own XWayland), so without X11 handheld mode cannot open
a window at all. The cost: X11 apps on the same display can read each other's
keystrokes. In Game Mode, that means Crossparty and the games running beside it
could see each other's key presses, including anything typed into the
Microsoft sign-in page on the on-screen keyboard. Sign in on the desktop
instead: the sign-in is kept (SEC-009), so Game Mode never needs it typed.
Games under Gamescope already share that one X11 display with each other, so
this adds Crossparty to an existing shared space rather than opening a new one.
Handheld mode itself is chosen from environment variables only
(`XDG_CURRENT_DESKTOP=gamescope` or `GAMESCOPE_WAYLAND_DISPLAY`;
`CROSSPARTY_HANDHELD=1` or `0` overrides), and it turns off the overlay,
corner notices and background holder, which need the Wayland layer Gamescope
does not have.

X11 also needs `--share=ipc`: X11 passes images through shared memory, and
Flathub rejects X11 without it. This shares the host's System V IPC namespace
(shared memory and semaphores) with the app, not files or sockets; it is the
standard pairing on Flathub and adds little beyond X11 itself
_(amended 2026-11-06)_.

**Tests:** `tests/GameModeDetectionTest.cpp` checks the detection and the
override. The X11 fallback is manual: nested Gamescope on the desktop (see
docs/plan.md, Handheld testing without a handheld).

### SEC-020: CodeQL scans the code and workflows

**Decision:** A CodeQL workflow analyses the C++ code and the GitHub Actions
workflows on every push and pull request, and once a week, with the
`security-extended` rule set. It reads the source without building it.

**Why:** CodeQL finds whole kinds of security bugs that compiler warnings,
clang-tidy and clazy do not, such as untrusted data reaching a file path or a
command, and unsafe patterns in workflows such as untrusted pull request text
used in a shell step. The weekly run applies new rules to code that has not
changed. Building is skipped because Qt 6.11 and QtWebEngine exist only in the
Flatpak SDK, not on GitHub's runners; CodeQL then works from the source alone,
which can miss a little that a full build would show.

**Permissions:** this job is the one exception to SEC-003's read-only token:
it gets `security-events: write`, which can only upload scan results to the
repo's Security tab. It still cannot push code or change releases. The actions
are pinned by commit hash (SEC-002).

**Tests:** the workflow itself, on every push. Results are reviewed in the
Security tab, and any real finding is fixed before a release.

### SEC-021: GitHub settings are code, applied by hand with a short-lived token

**Decision:** The repository's GitHub settings live in `infra/github` as
OpenTofu code:

- `main` and `staging` change only through pull requests whose CI and CodeQL
  checks passed, with no force pushes or deletion and no bypass, the owner
  included. The checks must come from the GitHub Actions app.
- `v*` release tags cannot be moved or deleted.
- Secret scanning with push protection, Dependabot alerts and Dependabot
  security fixes are on.
- Merged branches are deleted.

The state file is encrypted with OpenTofu's client-side encryption (PBKDF2
from a passphrase, then AES-GCM) and kept out of the repo. Applies use a
fine-grained token limited to this repository and to Administration, made
for one day and deleted afterwards. The token and passphrase are read from
the terminal without echo, never stored in files or shell history, and never
given to CI.

**Why:** Settings changed by clicking leave no record and can drift without
anyone noticing; as code, every change is reviewed in a pull request and
`tofu plan` shows any drift. The rulesets make a stolen session or a bad
push much less damaging: nothing reaches `main` or `staging` without passing
the checks, and a released tag can never be pointed at different code. The
admin token is the most powerful secret in the project, so it is made only
when needed, limited to one repository, and short-lived. The state file
records settings and IDs, not secrets, but it is encrypted anyway, so a copy
of it shows nothing. The provider is pinned to one version, with its
checksums in `.terraform.lock.hcl`; the repository has `prevent_destroy` and
`archive_on_destroy`, so no mistaken command can delete it.

**Not covered:** private vulnerability reporting, which the provider cannot
manage, is turned on by hand once (infra/github/README.md).

**Tests:** `tofu fmt` runs in dev/lint.sh; `tofu validate` was run when the
config was written. Each apply is checked by reading its plan first.

### SEC-022: Weekly checks run signed out, and only a separate job can open issues

**Decision:** A scheduled workflow runs weekly:

- `tools/PageCheck` loads Microsoft's page signed out at the overlay's narrow
  width. It fails if the page goes to a host outside the allowlist (watching
  for 10 more seconds after the Guide button appears, to catch late
  redirects), or if the Guide button the invite watcher reads is gone. It
  stops itself after 150 seconds even if the page never answers, and each
  job has a timeout, so a stuck run cannot hold a runner for hours.
- `dev/check-updates.sh` compares the manifest with the newest KDE runtime,
  QtWebEngine base app and layer-shell-qt.

A failure opens a GitHub issue, or comments on the open one.

**Why:** Microsoft can change its page, and upstream projects can ship
fixes, without anything changing here. A weekly check finds out before users
do. The checks run signed out because a signed-in check would need a
Microsoft account's password or tokens stored in CI.

**Permissions:** opening issues needs `issues: write`, a second exception to
SEC-003's read-only token. Only the reporting job gets it, and that job
checks out nothing, runs none of the repository's code and never reads the
checks' output, so nothing from Microsoft's page or the network can reach a
step that holds the write token. Its issue text is fixed, with a link to
the run.

**Sandbox:** Chromium's own sandbox cannot start inside a Docker container,
so the page check turns it off (`QTWEBENGINE_DISABLE_SANDBOX=1`). Only this
CI job does: it loads only Microsoft's page, signed out, holds no secrets
and runs on a throwaway machine. The app always runs with the sandbox on
(SEC-012).

**Tests:** both checks were run against the live services when written, and
`dev/check-updates.sh` was also run against an older runtime version to see
it fail. `tests/SiteAllowlistTest.cpp` checks that the start page shared by
the app and the page check is on the allowlist.

### SEC-023: Releases are built only in CI, from main, and signed

**Decision:** Pushing a `v*` tag runs `.github/workflows/release.yml`:

1. A quick read-only job checks the tagged commit is on `main`, so a tag
   pushed anywhere else is refused in seconds, before the long build.
2. A read-only job checks the tag matches the version in CMakeLists.txt, the
   metainfo and CHANGELOG.md (`dev/release-notes.sh`), then builds the
   Flatpak bundle from scratch, with no build cache.
3. A third job checks again that the commit is on `main`, then publishes the
   bundle, its SHA-256 checksum and a build provenance attestation as a
   GitHub release.

**Why:** A release built on a personal machine can only be trusted as far
as that machine. Built in CI from a tagged commit on `main`, every release
traces back to reviewed code that passed the checks (SEC-021). The
attestation is signed with GitHub's short-lived identity for this workflow
run, so `gh attestation verify` can prove which repository, workflow and
commit produced the file; the checksum catches a damaged or swapped download.
No cache, because a cache saved by any branch's build could otherwise feed
into a release. Release tags cannot be moved or deleted (SEC-021), so a
version always means the same code.

**Permissions:** only the publishing job can write. It gets `contents:
write` to create the release, and `id-token: write` and `attestations: write`
to sign the attestation. It builds nothing: it only checks, downloads the
bundle the read-only job built, and publishes it. The actions are pinned by
commit hash (SEC-002).

**Tests:** `dev/release-notes.sh` was run for a ready version, a wrong
version and a missing CHANGELOG section. The workflow itself first runs on
the v0.1.0 tag.

### SEC-024: Controllers work while Crossparty is focused, and only then

**Decision:** Crossparty reads controllers itself with SDL3, from the KDE
runtime, and turns them into the keys its interface and Microsoft's page
already understand: the D-pad and left stick become arrow keys, A becomes
Enter, and B becomes Escape, except on Microsoft's page, which has no key
for "back": there B goes back in the page's history, like a browser's Back
button. In handheld mode, Menu (≡) moves focus between the page and
Crossparty's own Mute and Sign Out bar, because the page keeps the arrow
keys for itself; it is a call into Crossparty's interface, never a key the
page sees. It reads controllers only while Crossparty is
the active app. When another app comes to the front, it stops reading; when
Crossparty comes back, everything pressed meanwhile is read and thrown away
before anything is sent. The Guide button and every button not named here
are ignored. Controller names are logged when they connect; presses never are.

**Why:** Steam Input's usual layout for a non-Steam app sends gamepad input,
which Crossparty did not read, so handheld mode needed manual remapping
(README, Controller setup, in 0.1.0). The spike (`docs/spike-controller.md`)
found an easier route and rejected it: with a read-only `/run/udev`,
Chromium's Gamepad API lets play.xbox.com drive itself with a controller, but
Chromium gave the page every press even while Crossparty was not focused or
was minimised. Whether a stray A press in a game ever reached a button in
the party would then depend on Microsoft's page, and Microsoft's code could
see everything pressed in games. Reading the controller in Crossparty's own
code puts the focus rule where the app controls it, and the page only ever
receives ordinary keys.

**Permissions:** `--device=input` (the devices in `/dev/input`), with
`--require-version=1.16.0`, the first stable Flatpak series to know it
(Flathub requires that); the fallback
Flathub offers for older versions is `--device=all`, which grants every
device, so older Flatpak installs are refused instead. Not
`/run/udev`: without udev's database Chromium finds no controllers, so the
page cannot use the Gamepad API even though the devices are readable. SDL is
told not to look for raw USB devices (hidraw), which the app cannot open
anyway.

**Fail safe:** if SDL cannot start, controllers are simply off and the app
works as before. Keys are only ever sent to a focused Crossparty window.
Going back never reloads the party page: Microsoft's app changes pages
without loading a new document, so its back steps never reach the
navigation check, and one that would load a whole page (for example back
into the sign-in pages) is refused, because a reload ends the party. This
also covers a mouse's Back button.

**Tests:** `tests/GamepadNavigatorTest.cpp` (which keys each button and
stick position becomes, repeating, letting go, and forgetting held
directions when focus is lost). Checked by hand, on the desktop and in
nested Gamescope: the D-pad, stick, A and B drive the page and the
handheld bar; Menu moves between them, and B closes the sign-out dialog;
B steps back through the page to its
start without a reload; presses made with another window focused, and with
Crossparty minimised, did nothing.

## Accepted risks

### RISK-001: QtWebEngine gets Chromium security fixes later than Chrome

QtWebEngine is patched through the KDE runtime, which lags Chrome's releases.
Exposure is limited because only Microsoft's own origins ever load in the app,
and every other link opens in your default browser. This risk is reviewed at
each release.

### RISK-002: Software running as you can reach the saved sign-in

The cookie file is encrypted (SEC-009), but its key is in KWallet. Any
program running as you while KWallet is unlocked could ask the Secret portal
or KWallet for secrets, as it could for every other keyring entry. Flatpak
gives each app its own secret, so another Flatpak app cannot ask for
Crossparty's; an unsandboxed program can. Protecting against software already
running as you is outside what an app can do, so this is accepted.

### RISK-003: X11 keystrokes are shared in Steam's Game Mode

In Game Mode, Crossparty and the games beside it share Gamescope's X11
display, so each could read the other's key presses (SEC-019). Signing in on
the desktop, where Crossparty uses Wayland only, avoids typing the password in
Game Mode at all. Accepted because handheld mode cannot work without it, and
the games already share that display with each other.

### RISK-004: The controller permission covers every input device

Flatpak's `--device=input` grants all of `/dev/input`, which includes
keyboards and mice as well as controllers, and it applies whether or not
Crossparty is focused. Crossparty's code opens controllers only and reads
them only while it is the active app (SEC-024), but the sandbox does not
enforce that. Accepted because it is the narrowest permission Flatpak has
for controllers, Flathub accepts it for exactly this, and handheld mode
needs controllers to work without setup. Microsoft's page is kept from
controllers separately, by leaving out `/run/udev`.
