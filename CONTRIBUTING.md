# Contributing

## Setting up

Run `dev/setup.sh` once on the host, then do all work inside the container:

```bash
distrobox enter crosstalk-dev
```

Commit from inside the container. The git hooks call tools that exist only
there, so a commit made from the host fails on purpose.

## Code style

- Follow KDE's coding style for C++ and QML. Formatting is automatic
  (`clang-format`, `qmlformat`) and runs on every commit.
- Names say what things are: `overlayWindow`, `isMuted`, `allowedOrigins`.
  No abbreviations and no single letters outside short loops.
- Keep functions small and doing one thing. If a function needs a comment
  to explain what it does, split it or rename it.
- Comments explain why, not what. Every security decision links to its entry
  in `docs/security.md`.
- One responsibility per file. Constants are named in one place.
- No dead code and no commented-out code.
- Fail safe: deny, close and log rather than carry on.

## Running the checks

| Command | What it does |
|---|---|
| `dev/lint.sh` | Formatters, QML lint, secret scan, clazy, clang-tidy, unit tests |
| `dev/flatpak-build.sh` | Builds the Flatpak, runs the tests inside it, installs it |
| `flatpak run io.github.KreatureofKreation.Crosstalk` | Runs the installed build |

## Workflow

1. Branch from `staging`.
2. Open a small pull request into `staging`, with a clear description. Nothing
   is pushed straight to `main`.
3. Write commit messages in plain English: what changed and why.
4. Go through the review checklist in the pull request template.
5. `staging` is merged into `main` only after the manual test checklist passes.
