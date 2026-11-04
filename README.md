# Crosstalk

Cross-platform party chat for Linux.

Crosstalk is an unofficial desktop app for Xbox party chat on the web. It
wraps the official web app at xbox.com/play in a native-feeling KDE window,
with an in-game overlay and a handheld mode.

**Status:** stage 0 (project setup). There is nothing to use yet.

## Disclaimer

Crosstalk is an unofficial project and is not affiliated with, endorsed by, or
sponsored by Microsoft. Xbox is a trademark of Microsoft Corporation.

Crosstalk comes with no warranty (see [LICENSE](LICENSE)). It is a hobby
project, and fixes are best effort.

## Privacy

Crosstalk collects nothing. There is no telemetry, analytics or crash
reporting. The only network traffic is to Microsoft's own sites, the same as
using them in a browser. Microsoft still sees everything said and done in a
party.

## Development

You need a Linux desktop with Podman, Distrobox and Flatpak (Bazzite has all
three). From the repository root:

```bash
dev/setup.sh
```

```bash
distrobox enter crosstalk-dev
```

Then inside the container:

```bash
dev/lint.sh
```

```bash
dev/flatpak-build.sh
```

See [CONTRIBUTING.md](CONTRIBUTING.md) for the code style and workflow, and
[docs/security.md](docs/security.md) for why things are the way they are.

## Licence

MIT. Qt and QtWebEngine are LGPL. They are supplied unmodified by the Flatpak
runtime and dynamically linked.
