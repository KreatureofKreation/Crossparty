# Contributing

## Setting up

Run `dev/setup.sh` once on the host, then do all work inside the container:

```bash
distrobox enter crossparty-dev
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

## Interface text

Every piece of text a user can see is wrapped for translation: `qsTr("...")`
in QML, `tr("...")` in C++. Log messages are not, because they are for
developers. Qt's own system (Qt Linguist) is used rather than KDE's KI18n:
it needs no extra library, and the app is not part of KDE's translation
infrastructure.

## Settings

Settings live in one file, `settings.ini` in the app's config folder, read and
written only through `src/core/SettingsFile.cpp`. The file carries a `version`
number. Changing what a key means, or renaming one, needs a new version and a
conversion from the old one in `SettingsFile::load`, with a test. A file with
a newer version than the app knows is left alone.

## Running the checks

| Command | What it does |
|---|---|
| `dev/lint.sh` | Formatters, QML lint, secret scan, clazy, clang-tidy, unit tests |
| `dev/flatpak-build.sh` | Builds the Flatpak, runs the tests inside it, checks it with Flathub's linter, installs it |
| `flatpak run io.github.KreatureofKreation.Crossparty` | Runs the installed build |

## Workflow

1. Branch from `staging`.
2. Write commit messages in plain English: what changed and why.
3. Push the branch and open a small pull request into `staging`, with a clear
   description, and go through the review checklist in the template.
4. The pull request can be merged once its four checks pass: lint and unit
   tests, the Flatpak build, and CodeQL for C++ and for the workflows. GitHub
   refuses direct pushes, force pushes and deletion on `staging` and `main`,
   for everyone (docs/security.md#sec-021).
5. Merge with a merge commit, so each commit's message stays in the history.
6. `staging` is merged into `main`, by pull request, only after the manual
   test checklist in docs/plan.md passes. Releases are tagged on `main`, and
   a release tag can never be moved.

## Repository settings

GitHub's settings are code in `infra/github`, applied by hand
(infra/github/README.md). Change them there, not in the web interface.

## Flathub's linter

`dev/flatpak-build.sh` and CI run `flatpak-builder-lint`. The few accepted
exceptions are in `dev/flatpak-lint-exceptions.json`, each with its reason.
Add one only with a reason, and remove it as soon as it is fixed.
