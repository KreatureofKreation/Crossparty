#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT
#
# Checks a site made by dev/flatpak-repo.sh the way a user's Flatpak will see
# it: adds the repository from its .flatpakrepo file, with only the signing
# key it carries, into a throwaway Flatpak installation, then lists and
# installs the app. Flatpak refuses an unsigned or wrongly signed repository,
# so this fails unless every signature checks out. The release workflow runs
# it before publishing anything.
#
#     dev/flatpak-repo-verify.sh site

set -euo pipefail

readonly app_id="io.github.KreatureofKreation.Crossparty"

site="$(realpath "${1:?Usage: dev/flatpak-repo-verify.sh <site directory>}")"
work="$(mktemp -d "$(dirname "$site")/.verify-XXXXXX")"
trap 'rm -rf "$work"' EXIT

# The same file users get, pointed at the files on disk instead of the site.
sed "s|^Url=.*|Url=file://$site/repo/|" "$site/crossparty.flatpakrepo" > "$work/local.flatpakrepo"

# Only ever the throwaway installation, never yours. Inside Distrobox,
# flatpak is the host's and does not see this shell's environment, so the
# setting is handed over with the command.
check_flatpak() {
    local command=(env FLATPAK_USER_DIR="$work/installation" flatpak --user "$@")
    if [[ -n "${CONTAINER_ID:-}" ]] && command -v distrobox-host-exec > /dev/null; then
        distrobox-host-exec "${command[@]}"
    else
        "${command[@]}"
    fi
}

check_flatpak remote-add crossparty-check "$work/local.flatpakrepo"
check_flatpak remote-ls --app crossparty-check | grep -q "$app_id"
# Without the runtime: this only checks the app arrives, signed.
check_flatpak install --noninteractive --no-deps crossparty-check "$app_id"
check_flatpak info "$app_id" | grep -iE "^ *(ref|commit):"

echo "Signed repository verified: $site"
