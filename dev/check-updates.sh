#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT
#
# Part of the weekly scheduled checks (docs/plan.md, Scheduled checks).
# Compares what the Flatpak manifest is built on with the newest releases:
# the KDE runtime, Flathub's QtWebEngine base app, and layer-shell-qt. Exits
# with 1 and lists them when something newer is waiting. Changes nothing.
#
#     dev/check-updates.sh

set -euo pipefail

readonly manifest="io.github.KreatureofKreation.Crossparty.yml"
readonly repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly layer_shell_repository="https://invent.kde.org/plasma/layer-shell-qt.git"

cd "$repository_root"

# The value of a top-level "key: value" line in the manifest, without quotes.
manifest_value() {
    sed -n "s/^$1: *'\{0,1\}\([^']*\)'\{0,1\}$/\1/p" "$manifest"
}

# The newest of a list of version numbers, one per line.
newest() {
    sort --version-sort | tail -n 1
}

# The branches Flathub has for a runtime or base app, one per line. Only
# numbered branches count; "stable" and "beta" names are skipped.
flathub_branches() {
    flatpak remote-ls --user --all --columns=ref flathub \
        | sed -n "s#^runtime/$1/x86_64/\([0-9][0-9.]*\)\$#\1#p;s#^app/$1/x86_64/\([0-9][0-9.]*\)\$#\1#p"
}

if ! flatpak remotes --user --columns=name | grep -qx flathub; then
    flatpak remote-add --user --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
fi

updates=()

check() {
    local name="$1" current="$2" latest="$3"
    if [[ -z "$latest" ]]; then
        echo "Could not find any release of $name"
        exit 2
    fi
    if [[ "$current" != "$latest" ]] && [[ "$(printf '%s\n%s\n' "$current" "$latest" | newest)" == "$latest" ]]; then
        updates+=("$name: $current -> $latest")
    fi
    echo "$name: using $current, newest $latest"
}

check "KDE runtime (org.kde.Platform)" \
    "$(manifest_value runtime-version)" \
    "$(flathub_branches org.kde.Platform | newest)"

check "QtWebEngine base app (io.qt.qtwebengine.BaseApp)" \
    "$(manifest_value base-version)" \
    "$(flathub_branches io.qt.qtwebengine.BaseApp | newest)"

check "layer-shell-qt" \
    "$(sed -n 's/^ *tag: v\(.*\)$/\1/p' "$manifest" | head -n 1)" \
    "$(git ls-remote --tags --refs "$layer_shell_repository" 'v*' | sed 's#.*refs/tags/v##' | grep -E '^[0-9]+(\.[0-9]+)*$' | newest)"

if ((${#updates[@]} > 0)); then
    echo
    echo "Updates waiting:"
    printf '  %s\n' "${updates[@]}"
    exit 1
fi
echo "Everything is up to date."
