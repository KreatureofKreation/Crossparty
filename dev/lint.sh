#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT
#
# Runs every formatter, linter and scanner. Run it inside the dev container
# (CI runs this same script in the same image):
#
#     dev/lint.sh

set -euo pipefail

readonly repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly build_directory="build/lint"

cd "$repository_root"

echo "==> Formatting, QML lint and secret scan (pre-commit hooks)"
pre-commit run --all-files --show-diff-on-failure

echo "==> Secret scan of the whole git history"
gitleaks git --redact --no-banner

echo "==> Compiling with clazy (Qt-specific checks); any warning fails"
CLAZY_CHECKS="level1" cmake -S . -B "$build_directory" -G Ninja \
    -DCMAKE_CXX_COMPILER=clazy \
    -DCMAKE_COMPILE_WARNING_AS_ERROR=ON \
    -DBUILD_TESTING=ON
cmake --build "$build_directory"

echo "==> clang-tidy"
# Only our own sources: the regex skips files Qt generates in the build folder.
run-clang-tidy -quiet -p "$build_directory" "^${repository_root}/(src|tests)/"

echo "==> Unit tests"
ctest --test-dir "$build_directory" --output-on-failure
