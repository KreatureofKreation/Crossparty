# GitHub settings

The repository's settings on GitHub, as OpenTofu code: branch and tag
rulesets, required checks, secret scanning with push protection, and
Dependabot alerts and security fixes. Why each one is set is in the comments
of the `.tf` files and in [docs/security.md](../../docs/security.md#sec-021).

Settings are changed here and applied by hand, never in the GitHub web
interface and never from CI. `tofu plan` also shows any change made in the
web interface since the last apply.

## What you need

- **A GitHub token, made for this one apply.** A fine-grained personal access
  token with:
  - Repository access: only `KreatureofKreation/Crossparty`
  - Repository permissions: **Administration: Read and write** (everything
    else stays at No access; Metadata: Read is added automatically)
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

## One manual setting

The OpenTofu provider cannot manage **private vulnerability reporting**,
which [SECURITY.md](../../SECURITY.md) asks reporters to use. Turn it on once
in the repository's Settings, Security, "Private vulnerability reporting".

## The state file

`terraform.tfstate` stays in this folder, encrypted, and is never committed.
If it or the passphrase is lost, nothing on GitHub changes: delete the file,
add `import` blocks for the two rulesets (their IDs are in the repository's
Settings, Rules), and plan again.
