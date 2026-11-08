#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT
#
# Turns a release bundle into Crossparty's signed Flatpak repository, the site
# that GitHub Pages serves: the repository itself, the .flatpakrepo and
# .flatpakref files that point Flatpak at it, the public signing key, and a
# short page with the install commands. See docs/security.md#sec-025.
#
#     FLATPAK_SIGNING_KEY="$(cat secret-key.asc)" \
#         dev/flatpak-repo.sh crossparty.flatpak site
#
# The secret key comes in FLATPAK_SIGNING_KEY, ASCII-armoured, and is only
# ever imported into a throwaway GnuPG home that is deleted on exit. It must
# be the secret half of data/release-signing.asc: signing with any other key
# fails, so a wrong secret can never publish a repository users would trust.
# CI runs this in the release workflow; it runs the same way locally with a
# test key (dev/flatpak-repo-test.sh).

set -euo pipefail

readonly app_id="io.github.KreatureofKreation.Crossparty"
readonly repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly public_key="${CROSSPARTY_PUBLIC_KEY:-$repository_root/data/release-signing.asc}"
# Where the site is served. Overridden only by the local test.
readonly site_url="${CROSSPARTY_SITE_URL:-https://kreatureofkreation.github.io/Crossparty}"

bundle="${1:?Usage: dev/flatpak-repo.sh <bundle.flatpak> <site directory>}"
site="${2:?Usage: dev/flatpak-repo.sh <bundle.flatpak> <site directory>}"
: "${FLATPAK_SIGNING_KEY:?FLATPAK_SIGNING_KEY must hold the ASCII-armoured secret signing key}"

bundle="$(realpath "$bundle")"
mkdir -p "$site"
site="$(realpath "$site")"
if [[ -n "$(ls -A "$site")" ]]; then
    echo "The site directory must be empty: $site" >&2
    exit 1
fi

# ostree and flatpak's repository tools are in the release container. On a
# desktop they come from Flatpak Builder, which can see the host's files.
run_tool() {
    if command -v ostree > /dev/null; then
        "$@"
    else
        flatpak run --command="$1" org.flatpak.Builder "${@:2}"
    fi
}

# Next to the site, not in /tmp: on a desktop the tools run in Flatpak
# Builder's sandbox, which has a /tmp of its own.
gnupg_home="$(mktemp -d "$(dirname "$site")/.gnupg-XXXXXX")"
cleanup() {
    run_tool gpgconf --homedir "$gnupg_home" --kill gpg-agent || true
    rm -rf "$gnupg_home"
}
trap cleanup EXIT
gpg_run() {
    run_tool gpg --homedir "$gnupg_home" --batch "$@"
}

# The fingerprint to sign with is the committed public key's, not whatever
# the secret happens to contain.
fingerprint="$(gpg_run --show-keys --with-colons "$public_key" | awk -F: '$1 == "fpr" { print $10; exit }')"
if [[ -z "$fingerprint" ]]; then
    echo "No key found in $public_key" >&2
    exit 1
fi
gpg_run --quiet --import <<< "$FLATPAK_SIGNING_KEY"
if ! gpg_run --list-secret-keys "$fingerprint" > /dev/null 2>&1; then
    echo "The signing secret is not the key in $public_key" >&2
    exit 1
fi

# A fresh repository holding only this release. Flatpak fetches only the
# files that changed, so users updating need no history from earlier
# releases, and nothing old lingers on the site.
repo="$site/repo"
run_tool ostree init --repo="$repo" --mode=archive-z2
run_tool flatpak build-import-bundle \
    --gpg-sign="$fingerprint" --gpg-homedir="$gnupg_home" \
    --no-update-summary \
    "$repo" "$bundle"
# The summary lists what the repository holds and is signed too; the
# AppStream data it builds is what Discover shows.
run_tool flatpak build-update-repo \
    --gpg-sign="$fingerprint" --gpg-homedir="$gnupg_home" \
    --title="Crossparty" \
    --default-branch=master \
    --generate-static-deltas \
    "$repo"

branch="$(run_tool ostree refs --repo="$repo" | awk -F/ -v id="$app_id" '$1 == "app" && $2 == id { print $4; exit }')"
if [[ -z "$branch" ]]; then
    echo "The bundle does not contain $app_id" >&2
    exit 1
fi

# Flatpak wants the public key as base64 of its binary form, on one line.
key_base64="$(gpg_run --dearmor < "$public_key" | base64 --wrap=0)"

cat > "$site/crossparty.flatpakrepo" << EOF
[Flatpak Repo]
Title=Crossparty
Url=$site_url/repo/
Homepage=https://github.com/KreatureofKreation/Crossparty
Comment=Unofficial Linux app for Xbox party chat
Description=Crossparty's own Flatpak repository, signed by its release workflow
GPGKey=$key_base64
EOF

# The runtime comes from Flathub, so the .flatpakref names it as the place
# to get it from.
cat > "$site/crossparty.flatpakref" << EOF
[Flatpak Ref]
Name=$app_id
Branch=$branch
Title=Crossparty
Url=$site_url/repo/
SuggestRemoteName=crossparty
RuntimeRepo=https://dl.flathub.org/repo/flathub.flatpakrepo
IsRuntime=false
Homepage=https://github.com/KreatureofKreation/Crossparty
GPGKey=$key_base64
EOF

cp "$public_key" "$site/release-signing.asc"
sed "s|@SITE_URL@|$site_url|g; s|@FINGERPRINT@|$fingerprint|g" \
    "$repository_root/data/site/index.html" > "$site/index.html"

echo "Signed repository for $app_id ($branch) in $site, key $fingerprint"
