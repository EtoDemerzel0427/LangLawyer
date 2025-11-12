# LangLawyer

Small C++ snippets exploring language features and build mechanics. The repository is organised into self-contained subdirectories and is wired up with CMake and Conan so you can reproduce builds across different machines.

## Requirements

- CMake 3.29+
- A C++20-capable compiler (e.g. clang, Apple clang, or GCC)
- [`uv`](https://docs.astral.sh/uv/) for Python toolchain management
- Git

Everything else (Python interpreter version, Conan) is pinned and set up automatically.

## 1. Create the Python tooling environment

The repo ships with `.python-version` and `python_env/pyproject.toml` that describe the exact tooling stack.

```bash
cd /Users/weiran/Desktop/CodeSpace/C++Code/LangLawyer/python_env
uv python install 3.12.2   # obeys the pinned interpreter
uv sync                    # creates python_env/.venv with conan==2.10.2

# Optional: drop into the virtualenv for interactive work
source .venv/bin/activate
```

From here you can run Conan either with the activated virtualenv or by prefixing commands with `uv run`.

## 2. Detect (or customise) a Conan profile

```bash
conan profile detect --force
```

This creates `~/.conan2/profiles/default`. Copy and tweak it if you want named profiles (for different compilers or architectures).

## 3. Install dependencies and generate the CMake toolchain

All commands below run from the repository root.

```bash
cd /Users/weiran/Desktop/CodeSpace/C++Code/LangLawyer

conan install . \
  --profile=default \
  --settings=build_type=Release \
  --build=missing \
  --output-folder=build/gcc-release
```

Conan places `conan_toolchain.cmake` and preset files under `build/gcc-release/`. Repeat with other `--profile` or `--settings` values as needed.

## 4. Configure and build with CMake

```bash
cmake -S . -B build/gcc-release \
  -DCMAKE_TOOLCHAIN_FILE=build/gcc-release/conan_toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build/gcc-release
```

Resulting binaries appear inside the build folder:

- `build/gcc-release/build_process/libbuild_process_lib.dylib`
- `build/gcc-release/build_process/build_process_app`
- `build/gcc-release/auto/concept_auto`
- `build/gcc-release/auto/decl_auto`
- `build/gcc-release/auto/auto_deduce`

Use `cmake --build … --target <name>` to build a specific target.

## 5. (Optional) Let Conan drive the build

Instead of calling CMake manually, you can run:

```bash
conan build . --output-folder=build/gcc-release
```

This uses the recipe in `conanfile.py` (which simply invokes CMake with the detected layout).

## Repository layout

- `auto/` – three C++ programs demonstrating `auto` behaviour (`concept_auto`, `decl_auto`, `auto_deduce`)
- `build_process/` – shared library + app showing symbol visibility and linkage
- `python_env/` – reproducible Python environment definition for tooling (`uv` + Conan)
- `.python-version` – pins Python 3.12.2 for all contributors

## Cleaning up

Delete the build directory of your choice to start fresh, e.g.:

```bash
rm -rf build/gcc-release
```

The `.venv/` directory under `python_env/` is disposable; rerun `uv sync` to recreate it.

## Troubleshooting

- If CMake can’t find the toolchain file, ensure `conan install` ran with the same `--output-folder` path you pass to the configure step.
- If Conan warns about compiler settings, open the profile under `~/.conan2/profiles/` and adjust the `settings.compiler` entries to match your toolchain.
- Re-run `uv sync` after changing `pyproject.toml` so the environment picks up the new dependencies.

