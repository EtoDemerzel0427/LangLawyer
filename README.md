# LangLawyer

Small C++ snippets exploring language features and build mechanics. The repository is organized into self-contained subdirectories and is wired up with CMake and Conan so you can reproduce builds across different machines.

## Requirements

- CMake 3.29+
- A C++20-capable compiler (tested with clang / Apple clang; GCC should work; MSVC is not currently supported)
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

## 2. Detect (or customize) a Conan profile

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

## 4. Build the project (recommended)

```bash
conan build . --build-folder=build/gcc-release
```

This runs the recipe’s `build()` method, which invokes CMake using the toolchain generated in the previous step.

Resulting binaries land inside the same build directory, already split by type:

- Executables → `build/gcc-release/bin/`
- Libraries → `build/gcc-release/lib/`

### Optional: drive CMake manually

If you want to run CMake yourself:

```bash
cmake -S . -B build/gcc-release \
  -DCMAKE_TOOLCHAIN_FILE=build/gcc-release/conan_toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build/gcc-release
```

Use `cmake --build … --target <name>` to build a specific target. Enable intermediate compile outputs by adding `-DENABLE_SAVE_TEMPS=ON`.

### Keeping intermediate compilation artefacts

To emit `.ii`, `.s`, and other files produced by `-save-temps=obj`, flip the Conan option when installing:

```bash
conan install . \
  --profile=default \
  --settings=build_type=Release \
  --build=missing \
  --output-folder=build/gcc-release \
  -o save_temps=True

conan build . --build-folder=build/gcc-release
```

The option maps to the `ENABLE_SAVE_TEMPS` CMake cache variable, so manual CMake invocations can also pass `-DENABLE_SAVE_TEMPS=ON`.

## 5. Recipe internals (`conanfile.py`)

- `exports_sources`: copies the top-level `CMakeLists.txt` plus the `auto/` and `build_process/` trees into Conan’s cache so builds are self-contained.
- `generators = "CMakeToolchain", "CMakeDeps"`: produces `conan_toolchain.cmake` (compiler flags, std, build options) and `Find*.cmake` modules so future `find_package()` calls resolve to Conan dependencies instead of system libraries.
- `cmake_layout(self)`: standard folders (`build/<profile>/bin`, `lib`, `generators`, etc.) that match CMake expectations and make `conan build` work without extra arguments.
- `build()`: spins up CMake configure + build with the generator files. When you run `conan build`, this is the method that executes.

## Repository layout

- `libs/` – internal libraries (e.g. `build_process_lib`)
- `snippets/auto/` – three programs demonstrating `auto` behaviour (`concept_auto`, `decl_auto`, `auto_deduce`)
- `snippets/build_process/` – executable using `build_process_lib` to illustrate linking and build artefacts
- `python_env/` – reproducible Python environment definition for tooling (`uv` + Conan)
- `toolchain/` – documentation and helper scripts (pre-commit hook, formatting guidance, [Conan & CMake deep dive](toolchain/conan-cmake.md), [Conan 1 vs 2 comparison](toolchain/conan-1-vs-2.md))
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

