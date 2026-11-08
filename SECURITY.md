# Security policy

## Reporting a vulnerability

Please report security issues privately through GitHub's **Report a
vulnerability** button on the repository's Security tab. Do not open a public
issue. This is a hobby project, so fixes are best effort, but security
reports come first.

## If a key or account is lost or compromised

These are the secrets that control the project, where each one lives, and
what to do if one is lost, stolen or might have been seen.

| Secret | Where it lives | What it controls |
|---|---|---|
| GitHub account (password and 2FA) | Password manager; 2FA device | Everything below |
| SSH key `id_ed25519_github` | The development PC, passphrase protected | Pushing branches (docs/security.md#sec-008) |
| OpenTofu admin token | Made for one apply and deleted the same day | Repository settings and rulesets (docs/security.md#sec-021) |
| OpenTofu state passphrase | Password manager | Reading the encrypted state file only |
| Release signing key (GPG, Ed25519) | GitHub's `release-signing` environment secret; an offline backup with its revocation certificate | Which updates installed copies accept (docs/security.md#sec-025) |

### First, for any of them

1. **Revoke** it at once (steps below).
2. **Rotate:** make a replacement and store it only in the password manager.
3. **Audit:** check GitHub's security log (Settings, Security log), recent
   commits on every branch, releases and tags, and the repository's settings.
   Run `tofu plan` in `infra/github` with a fresh token: any change it wants
   to make back is a setting someone else changed.
4. **Disclose:** if a release could have been tampered with, publish a GitHub
   security advisory saying which versions and why, and publish a clean
   release rebuilt by CI from a reviewed commit.

### GitHub account

1. Sign in from a trusted device and change the password.
2. Settings, Password and authentication: check the 2FA methods; remove any
   you do not recognise, and make new recovery codes.
3. Settings, Sessions: sign out every other session.
4. Check Settings, SSH keys, Personal access tokens, Applications and
   Developer settings for anything you did not add, and remove it.
5. If you cannot sign in at all, use GitHub's account recovery with your
   recovery codes, then do the above.

### SSH key

1. Settings, SSH and GPG keys: delete the key.
2. Delete `~/.ssh/id_ed25519_github` and its `.pub` file on the PC.
3. Make a new key as in docs/security.md#sec-008 (or move to a hardware key,
   which is the plan), add it to GitHub, and check `git push` works.

### OpenTofu admin token

Tokens are deleted after each apply, so normally there is none to lose. If
one was kept or might have been seen: Settings, Developer settings, Personal
access tokens, delete it. Then run the audit step above.

### OpenTofu state passphrase or state file

The state file holds settings and IDs, not secrets. If the passphrase is lost
or seen, delete `infra/github/terraform.tfstate`, choose a new passphrase,
and re-import (infra/github/README.md). Nothing on GitHub changes.

### Release signing key

A stolen key alone cannot reach users: updates come only from this
repository's GitHub Pages site, which only the release workflow can change.
With the key and the site, though, someone could ship an update every
installed copy accepts, so treat it as urgent.

1. Settings, Environments, `release-signing`: delete `FLATPAK_SIGNING_KEY`.
   No release can be signed until a new one is added.
2. Make a new key as in infra/github/README.md, replace
   `data/release-signing.asc` through a pull request, and add the new secret.
3. Publish a new release. Installed copies still trust only the old key, so
   they refuse it: the security advisory (step 4 above) tells users to
   remove the repository and add it again, with:
   `flatpak remote-delete --user crossparty` and the install command in the
   README. Their sign-in and settings are kept.
4. Keep the old key's revocation certificate with the advisory, for anyone
   who imported the key into GnuPG.

If the key is only lost (no backup) and not stolen, the steps are the same
without the urgency.

### A tampered release

1. Delete the affected release's downloads from GitHub. The tag cannot be
   moved (docs/security.md#sec-021).
2. Publish the advisory (step 4 above) and a new version built by CI.
3. If it reached the Flatpak repository, redeploy the site from the clean
   release, and if the signing key could be involved, rotate it (above).
