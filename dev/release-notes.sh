#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT
#
# Checks that a version is ready to release, then prints its CHANGELOG
# section as the release notes. The release workflow runs it for the pushed
# tag; run it yourself before tagging:
#
#     dev/release-notes.sh 0.1.0
#
# Ready means the version is the same in CMakeLists.txt, in the AppStream
# metainfo, and as a "## <version>" heading in CHANGELOG.md.

set -euo pipefail

readonly version="${1:?Usage: dev/release-notes.sh <version>, for example 0.1.0}"
readonly repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly metainfo="data/io.github.KreatureofKreation.Crossparty.metainfo.xml"

cd "$repository_root"

fail() {
    echo "Not ready to release $version: $1" >&2
    exit 1
}

project_version="$(sed -n 's/^project(Crossparty VERSION \([0-9.]*\) .*/\1/p' CMakeLists.txt)"
[[ "$project_version" == "$version" ]] \
    || fail "CMakeLists.txt says ${project_version:-nothing}"

grep -q "<release version=\"$version\" date=\"[0-9-]*\" type=\"stable\">" "$metainfo" \
    || fail "$metainfo has no stable <release> entry for it"

# The lines under "## <version>" (with or without a date after it), up to
# the next "## " heading.
notes="$(awk -v heading="## $version" '
    index($0, heading) == 1 && (length($0) == length(heading) || substr($0, length(heading) + 1, 1) == " ") { inside = 1; next }
    inside && /^## / { exit }
    inside { print }
' CHANGELOG.md)"
[[ -n "${notes//[[:space:]]/}" ]] || fail "CHANGELOG.md has no \"## $version\" section"

printf '%s\n' "$notes"
