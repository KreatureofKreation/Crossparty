#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT
#
# Brings a fresh machine to a working build. Run it once on the host (your
# Bazzite desktop), from the repository root:
#
#     dev/setup.sh
#
# Nothing is installed on the immutable host itself. Tools go into a Distrobox
# container, and the Flatpak builder is itself a sandboxed Flatpak.

set -euo pipefail

readonly container_name="crossparty-dev"
readonly image_name="localhost/crossparty-dev:latest"
readonly repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$repository_root"

for required_command in podman distrobox flatpak git; do
    if ! command -v "$required_command" > /dev/null; then
        echo "error: '$required_command' is not installed on this machine." >&2
        exit 1
    fi
done

if [[ ! -d .git ]]; then
    echo "error: this folder is not a git repository yet. Run 'git init -b main' first." >&2
    exit 1
fi

echo "==> Building the development container image"
podman build --tag "$image_name" --file dev/Containerfile dev

if distrobox list | grep --quiet --word-regexp "$container_name"; then
    echo "==> Container '$container_name' already exists"
    echo "    To rebuild it from the new image: distrobox rm $container_name && dev/setup.sh"
else
    echo "==> Creating the '$container_name' Distrobox container"
    distrobox create --name "$container_name" --image "$image_name" --yes
fi

echo "==> Installing the Flatpak builder (a sandboxed Flatpak, not a host package)"
flatpak --user remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
flatpak --user install --noninteractive flathub org.flatpak.Builder

echo "==> Installing the git hooks"
distrobox enter "$container_name" -- pre-commit install

echo
echo "Done. Next:"
echo "  distrobox enter $container_name    # work, commit and run tools in here"
echo "  dev/flatpak-build.sh                # build, test and install the Flatpak"
