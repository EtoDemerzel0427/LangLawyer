#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

if [ ! -d "${repo_root}/.git" ]; then
  echo "Error: ${repo_root} is not a Git repository." >&2
  exit 1
fi

hook_source="${repo_root}/toolchain/hooks/pre-commit"
hook_destination="${repo_root}/.git/hooks/pre-commit"

if ! command -v clang-format >/dev/null 2>&1; then
  echo "Warning: clang-format not found in PATH. The pre-commit hook will fail until it is installed." >&2
fi

chmod +x "${hook_source}"
mkdir -p "$(dirname "${hook_destination}")"
ln -sf "${hook_source}" "${hook_destination}"

echo "Pre-commit hook installed at .git/hooks/pre-commit (symlinked to toolchain/hooks/pre-commit)."