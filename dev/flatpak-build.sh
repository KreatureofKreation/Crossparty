#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT
#
# Builds the Flatpak, runs the unit tests inside it and installs it for your
# user. It works from the host or from inside the dev container. Run it as:
#
#     dev/flatpak-build.sh
#     flatpak run io.github.KreatureofKreation.Crossparty

set -euo pipefail

readonly manifest="io.github.KreatureofKreation.Crossparty.yml"
readonly repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$repository_root"

# Flatpak builds run on the host. Inside Distrobox, hand the command over.
run_on_host() {
    if [[ -n "${CONTAINER_ID:-}" ]] && command -v distrobox-host-exec > /dev/null; then
        distrobox-host-exec "$@"
    else
        "$@"
    fi
}

run_on_host flatpak run org.flatpak.Builder \
    --user \
    --install \
    --force-clean \
    --install-deps-from=flathub \
    build-dir \
    "$manifest"
