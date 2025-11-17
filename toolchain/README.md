# Toolchain Overview

This directory documents the project’s build and formatting toolchain so everyone on the team follows the same workflow.

## Python Tooling (Conan)

- Python version pinned with `.python-version` → 3.12.2.
- Virtual environment described by `python_env/pyproject.toml` (managed with [`uv`](https://docs.astral.sh/uv/)).

### Setup

```bash
cd /Users/weiran/Desktop/CodeSpace/C++Code/LangLawyer/python_env
uv python install 3.12.2
uv sync
source .venv/bin/activate          # optional, for interactive shell
```

### Conan usage

Conan is already installed in the environment. Standard workflow from repo root:

```bash
conan profile detect --force
conan install . \
  --profile=default \
  --settings=build_type=Release \
  --build=missing \
  --output-folder=build/gcc-release
conan build . --build-folder=build/gcc-release
```

Pass `-o save_temps=True` during the install step to surface intermediate compilation artefacts. Use different folders or profiles to target other compilers/configurations.

## CMake Layout

- Top-level `CMakeLists.txt` sets policies, adds `libs/`, then `snippets/`, and exposes the `ENABLE_SAVE_TEMPS` cache option.
- `libs/build_process` defines `build_process_lib` and installs headers/targets.
- `snippets/auto` builds the `concept_auto`, `decl_auto`, and `auto_deduce` executables.
- `snippets/build_process` builds `build_process_app`, linking against `build_process_lib`.
- Conan’s `cmake_layout()` places generated toolchain files in `build/<profile>/`.

### Presets

Conan generates `CMakeUserPresets.json` pointing to the presets under `build/.../generators/`. You can open the project in IDEs that support CMake presets (CLion, VSCode with CMake Tools, etc.) and select the Conan-generated preset.

## Formatting (clang-format)

- `.clang-format` at the repo root enforces a consistent style (`BasedOnStyle: LLVM`, column limit 100, 4-space indents).
- Suggested manual usage:

```bash
clang-format -i snippets/auto/*.cpp snippets/build_process/*.cpp libs/build_process/*.cpp libs/build_process/*.hpp
```

- Pre-commit hook: run once to enable automatic formatting before every commit:

```bash
bash toolchain/hooks/setup.sh
```

The hook formats staged C/C++ sources with `clang-format -style=file` and re-stages them.

## Further reading

- [conan-cmake.md](conan-cmake.md) – in-depth explanation of the Conan recipe, generators, CMake targets, and how options propagate through the build.
- [conan-1-vs-2.md](conan-1-vs-2.md) – side-by-side comparison of the Conan 1 and Conan 2 workflows, including native `conan build` usage.

## Future Extensions

- Add `clang-tidy` configuration for static analysis.
- Define additional environment profiles (`profiles/clang15`, `profiles/gcc13`, etc.) and reference them here.
- Integrate CI scripts that call `uv sync`, `conan install`, and `cmake --build` to validate changes automatically.

