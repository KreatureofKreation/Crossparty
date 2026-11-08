# GitHub settings

The repository's settings on GitHub, as OpenTofu code: branch and tag
rulesets, required checks, secret scanning with push protection, Dependabot
alerts and security fixes, GitHub Pages and the two tag-only environments for
the signed Flatpak repository. Why each one is set is in the comments
of the `.tf` files and in [docs/security.md](../../docs/security.md#sec-021).

Settings are changed here and applied by hand, never in the GitHub web
interface and never from CI. `tofu plan` also shows any change made in the
web interface since the last apply.

## What you need

- **A GitHub token, made for this one apply.** A fine-grained personal access
  token with:
  - Repository access: only `KreatureofKreation/Crossparty`
  - Repository permissions: **Administration**, **Environments** and
    **Pages**, each Read and write (everything else stays at No access;
    Metadata: Read is added automatically)
  - Expiration: 1 day
- **The state passphrase**, at least 16 characters, kept in your password
  manager. It encrypts the state file. Use the same one every time; a lost
  passphrase means re-importing (see below).

Neither is ever written to a file or typed on a command line.

## Apply

From the repository root, inside the dev container:

```bash
cd infra/github
tofu init
read -rsp "GitHub token: " GITHUB_TOKEN && export GITHUB_TOKEN && echo
read -rsp "State passphrase: " TF_VAR_state_passphrase && export TF_VAR_state_passphrase && echo
tofu plan -out=settings.tfplan
```

Read the plan before going on. The first time, it should show:

- 3 to import: the repository, Dependabot alerts and Dependabot security
  fixes
- 2 to add: the two rulesets
- changes to the repository only for the description, deleting merged
  branches and secret scanning

Anything to destroy, or any other change, means stop and look. If it is
right:

```bash
tofu apply settings.tfplan
rm settings.tfplan
unset GITHUB_TOKEN TF_VAR_state_passphrase
```

Then delete the token on GitHub (Settings, Developer settings, Personal
access tokens).

### Adding Pages and the signing environments _(2026-11-08)_

The plan for this change should show 5 to add and nothing else: Pages
(`github_repository_pages`), the `release-signing` and `github-pages`
environments, and a `v*` tag policy for each. GitHub may already have made
`github-pages` when Pages was turned on; OpenTofu takes it over either way.

## The release signing key

Made once, by hand, on your own machine, never in CI or here in the
container. It signs every update users install (docs/security.md#sec-025).
On the host, from the repository root:

```bash
umask 077
export GNUPGHOME="$(mktemp -d)"
gpg --batch --passphrase '' --quick-generate-key "Crossparty release signing" ed25519 sign never
gpg --armor --export > data/release-signing.asc
```

**Back it up offline** before anything else: the secret key and the
revocation certificate. They are written into the throwaway GnuPG home, never
into the repository:

```bash
gpg --armor --export-secret-keys > "$GNUPGHOME/crossparty-release-secret.asc"
cp "$GNUPGHOME"/openpgp-revocs.d/*.rev "$GNUPGHOME/crossparty-release-revocation.rev"
echo "$GNUPGHOME"
```

Copy those two files from the folder it prints into your password manager
(as file attachments) or onto an encrypted USB stick. Without the backup, a
lost secret means every user has to re-add the repository.

**Add it to GitHub**, once the apply above has made the environment:
Settings, Environments, `release-signing`, Add environment secret, name
`FLATPAK_SIGNING_KEY`. Copy the secret straight to the clipboard, paste it
into the value box, then clear the clipboard:

```bash
gpg --armor --export-secret-keys | wl-copy
wl-copy --clear
```

**Then remove every local copy**:

```bash
gpgconf --kill gpg-agent
rm -rf "$GNUPGHOME"
unset GNUPGHOME
```

Commit `data/release-signing.asc` (the public half only) through a pull
request. `dev/flatpak-repo.sh` signs only with the key whose public half is
committed, so the secret and the file must match.

## One manual setting

The OpenTofu provider cannot manage **private vulnerability reporting**,
which [SECURITY.md](../../SECURITY.md) asks reporters to use. Turn it on once
in the repository's Settings, Security, "Private vulnerability reporting".

## The state file

`terraform.tfstate` stays in this folder, encrypted, and is never committed.
If it or the passphrase is lost, nothing on GitHub changes: delete the file,
add `import` blocks for the two rulesets (their IDs are in the repository's
Settings, Rules), and plan again.
