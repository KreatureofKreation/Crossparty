# Security policy

## Reporting a vulnerability

Please report security issues privately through GitHub's **Report a
vulnerability** button on the repository's Security tab. Do not open a public
issue. This is a hobby project, so fixes are best effort, but security
reports come first.

## If a key or account is lost or compromised

The secrets that matter are the GitHub account, the OpenTofu GitHub admin
token and, if used, the repository signing key.

1. **Revoke** the token or key right away, from GitHub settings or with GPG.
2. **Rotate:** create a replacement and store it only in the password manager.
3. **Audit:** check the GitHub security log, recent commits, releases and
   repository settings for changes you did not make. Re-run `tofu plan` to
   catch settings drift.
4. **Disclose:** if a release could have been tampered with, say so in a
   GitHub security advisory and publish a clean rebuilt release.

The full revoke-and-rotate plan is written up in stage 6.
