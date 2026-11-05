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

**Rejected:** keeping the normal on-disk profile (plain-text tokens readable by
any unsandboxed program running as the user), and switching to Electron (it
encrypts cookies but still keeps Local Storage in plain text, and it cannot
draw the overlay as a layer surface).

## Accepted risks

### RISK-001: QtWebEngine gets Chromium security fixes later than Chrome

QtWebEngine is patched through the KDE runtime, which lags Chrome's releases.
Exposure is limited because only Microsoft's own origins ever load in the app,
and every other link opens in your default browser. This risk is reviewed at
each release.
