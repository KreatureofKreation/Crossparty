#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT
#
# Tries dev/flatpak-repo.sh on this machine with a throwaway test key, never
# the real one: makes the key, builds the site from a bundle into
# build/repo-test, checks it with dev/flatpak-repo-verify.sh, and checks that
# a different secret key is refused. Run it from the host, as:
#
#     dev/flatpak-repo-test.sh crossparty.flatpak

set -euo pipefail

readonly repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
bundle="$(realpath "${1:?Usage: dev/flatpak-repo-test.sh <bundle.flatpak>}")"

test_dir="$repository_root/build/repo-test"
rm -rf "$test_dir"
mkdir -p "$test_dir"

# Test keys only, made in Flatpak Builder's GnuPG and kept in build/.
make_key() {
    local home="$test_dir/$1"
    mkdir -m 700 "$home"
    flatpak run --command=gpg org.flatpak.Builder --homedir "$home" --batch \
        --passphrase '' --quick-generate-key "Crossparty test key $1" ed25519 sign never
    flatpak run --command=gpg org.flatpak.Builder --homedir "$home" --batch --armor --export > "$test_dir/$1.asc"
    flatpak run --command=gpg org.flatpak.Builder --homedir "$home" --batch --armor --export-secret-keys > "$test_dir/$1-secret.asc"
    flatpak run --command=gpgconf org.flatpak.Builder --homedir "$home" --kill gpg-agent
}
make_key good
make_key other

export CROSSPARTY_PUBLIC_KEY="$test_dir/good.asc"
export CROSSPARTY_SITE_URL="file://$test_dir/site"

FLATPAK_SIGNING_KEY="$(cat "$test_dir/good-secret.asc")" \
    "$repository_root/dev/flatpak-repo.sh" "$bundle" "$test_dir/site"
"$repository_root/dev/flatpak-repo-verify.sh" "$test_dir/site"

if FLATPAK_SIGNING_KEY="$(cat "$test_dir/other-secret.asc")" \
    "$repository_root/dev/flatpak-repo.sh" "$bundle" "$test_dir/wrong-site"; then
    echo "FAILED: a secret key that is not the published key was accepted" >&2
    exit 1
fi
echo "Refused a secret key that is not the published key, as it should"

# And a user's Flatpak must refuse the repository when the key it trusts is
# not the one that signed it.
cp -r "$test_dir/site" "$test_dir/other-key-site"
other_base64="$(flatpak run --command=gpg org.flatpak.Builder --batch --dearmor < "$test_dir/other.asc" | base64 --wrap=0)"
sed -i "s|^GPGKey=.*|GPGKey=$other_base64|" "$test_dir/other-key-site/crossparty.flatpakrepo"
if "$repository_root/dev/flatpak-repo-verify.sh" "$test_dir/other-key-site"; then
    echo "FAILED: Flatpak accepted a repository signed by a key it does not trust" >&2
    exit 1
fi
echo "Flatpak refused a repository signed by a key it does not trust, as it should"
echo "All repository checks passed"
