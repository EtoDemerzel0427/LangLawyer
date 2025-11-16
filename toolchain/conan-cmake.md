# Conan & CMake Deep Dive

This guide explains how the LangLawyer project wires Conan and CMake together, what each generator does, and how the CMake targets are structured. Use it as a reference when you add new libraries/snippets or tweak the build.

---

## 1. Conan workflow

### 1.1 Recipe overview (`conanfile.py`)

- **`set_version()`** – runs `git describe --tags --dirty --always` so the package version matches your latest tag. Falls back to `0.0.0` if tags aren’t available.
- **`options/default_options`** – currently only `save_temps` (Boolean). Toggling it propagates to CMake’s `ENABLE_SAVE_TEMPS` cache variable.
- **`layout()`** – calls `cmake_layout(self)`, giving Conan a predictable folder structure under `build/<profile>/` (bin/, lib/, generators/, etc.).
- **`generate()`** – creates the toolchain and dependency files:
  - `CMakeToolchain` writes `conan_toolchain.cmake`, injecting compiler settings, build flags, and custom cache variables (e.g. `ENABLE_SAVE_TEMPS`).
  - `CMakeDeps` writes `Find<Package>.cmake` and `<Package>Config.cmake` files. Once you declare requirements, your `find_package()` calls resolve to the Conan-provided packages.
- **`build()`** – instantiates `CMake()`, runs `configure()` (consuming the generated toolchain/presets), then `build()`.
- **`package()`** – runs `cmake.install()` so the install rules in the project copy binaries, headers, and targets into the Conan package folder.

The build flow is therefore:

```
conan install … → generate toolchain/deps files → (optionally) cmake configure
conan build …   → reuse those files to configure & build in the requested folder
```

### 1.2 What is a generator?

In Conan, *generators* are small pieces of logic that translate package metadata into build-system friendly files or environment scripts. After `conan install`, they produce artifacts you can feed to your build tool—toolchain files, dependency graphs, command wrappers, etc. In Conan 2, generators live in Python and are configured per recipe (often via the `generate()` method).

- They do **not** build anything themselves. Their job is to prepare the environment for the build system.
- You can enable multiple generators; each one writes into `build/<profile>/generators/` (unless you override the destination).
- Typical examples: `CMakeToolchain`, `CMakeDeps`, `VirtualEnv`, `PkgConfigDeps`.

### 1.3 Conan 2 generators used here

| Generator       | Output                                      | Purpose in this project                                  |
|-----------------|---------------------------------------------|----------------------------------------------------------|
| `CMakeToolchain`| `conan_toolchain.cmake`, presets, env files | Pins compiler, standard, and platform flags; exports cache variables like `ENABLE_SAVE_TEMPS`; configures the CMake invocation. |
| `CMakeDeps`     | `Find<dep>.cmake`, `<dep>Config.cmake`, presets | Provides `find_package()` modules/config files so our project (and anything depending on the package) resolves Conan-managed dependencies. |

Both drop their files under `build/<profile>/generators/`, and the auto-generated `CMakeUserPresets.json` in the repo root includes those presets so IDEs can choose them.

### 1.4 How does this differ from Conan 1?

Conan 1 used legacy (mostly text-based) generators like:

| Conan 1 generator | Typical output                        | Notes |
|-------------------|---------------------------------------|-------|
| `cmake` / `cmake_multi` | `conanbuildinfo.cmake`                | A monolithic file that you had to `include()` manually. Provided functions like `conan_basic_setup()` which modified global CMake state. |
| `cmake_paths`     | Modified `CMAKE_MODULE_PATH`/`CMAKE_PREFIX_PATH` | Helped module-mode `find_package`, but required manual include and was easy to misconfigure. |
| `virtualenv`      | Activatable scripts (`activate.sh`, etc.) | Similar concept to Conan 2’s `VirtualEnv`, but less structured. |

Key differences between Conan 1 and Conan 2 generator ecosystem:

- **Separation of concerns**: Conan 2 splits toolchain configuration (`CMakeToolchain`) and dependency discovery (`CMakeDeps`), whereas Conan 1’s `cmake` generator mixed both in one file.
- **CMake presets/toolchains**: Conan 2 integrates with modern CMake (`CMakePresets.json`, cache variables), aligning with CMake’s multi-config and cross-building features. Conan 1 required more manual CMake logic.
- **Package layout awareness**: `cmake_layout()` and the `generate()` method make it easy to influence the build directories and generator behavior per recipe. In Conan 1, you often hard-coded folder structures or relied on environment variables.
- **Extensibility**: Conan 2 generators are Python classes you can configure or even subclass; Conan 1 generators were more rigid text templates.

If you encounter old instructions referencing `conanbuildinfo.cmake`, the modern equivalent is letting `CMakeToolchain` + `CMakeDeps` write the toolchain and dependency configs, then running CMake with the generated toolchain file.

### 1.3 Configuring builds with options/profiles

- **Options** – pass `-o langlawyer/*:save_temps=True` during `conan install` to enable intermediate artefacts. Each option maps to a CMake cache variable inside `generate()`.
- **Profiles** – capture compiler, architecture, build type, and environment values (e.g. `compiler=clang`, custom `LIBRARY_PATH`). Helpful when you want separate presets for Apple Clang vs Homebrew LLVM.
- **Build folders** – keep them separate per profile/configuration, e.g. `build/gcc-release`, `build/clang-debug`. Conan reuses the generated toolchain for the chosen folder.

Typical command sequence:

```bash
conan profile detect --force
conan install . \
  --profile=default \
  --settings=build_type=Release \
  --build=missing \
  --output-folder=build/gcc-release \
  -o langlawyer/*:save_temps=True
conan build . --build-folder=build/gcc-release
```

---

## 2. CMake structure

### 2.1 Top-level `CMakeLists.txt`

- Sets the language standard (C++20) and common policies.
- Defines `ENABLE_SAVE_TEMPS` (default OFF), so the option is discoverable from both Conan and manual CMake runs.
- Normalizes runtime/library/archive output directories to `bin/` and `lib/`.
- Adds subdirectories in this order:
  1. `libs/` – internal libraries available to snippets or future apps.
  2. `snippets/` – executables demonstrating the language/build features.

### 2.2 Libraries (`libs/`)

`libs/build_process/CMakeLists.txt` demonstrates several target properties:

- **`add_library(build_process_lib SHARED …)`** – builds the shared library from sources in `libs/build_process`.
- **`target_include_directories(... PUBLIC …)`** – uses generator expressions:
  - `$<BUILD_INTERFACE:…>` – include path used when building from source (points at the library’s directory).
  - `$<INSTALL_INTERFACE:include/...>` – include path consumers see after the library is installed or packaged.
  - **PUBLIC** means the include path is added for the library itself and for targets that link against it. Use:
    - **PRIVATE** when a compile option/include only applies to the target being defined.
    - **PUBLIC** when consumers should inherit it.
    - **INTERFACE** when the target itself doesn’t compile (header-only libraries); properties apply only to dependents.
- **`if(ENABLE_SAVE_TEMPS) target_compile_options(... PRIVATE -save-temps=obj)`** – toggles the flag only when the cache variable is ON.
- **`install(TARGETS …)`** – installs the built library, exports a `build_processTargets.cmake` file, and copies headers into `include/build_process`. This is what Conan’s `package()` step consumes.
- **`install(EXPORT … NAMESPACE langlawyer:: …)`** – exposes an imported target `langlawyer::build_process_lib` for downstream use via `find_package(langlawyer CONFIG)` (once the package is installed).

### 2.3 Snippets (`snippets/`)

- `snippets/auto` – builds simple executables and installs them into `bin/`.
- `snippets/build_process` – builds `build_process_app`, links against `build_process_lib`, and installs into `bin/`. The target honours `ENABLE_SAVE_TEMPS` so you can inspect the compilation pipeline on demand.

### 2.4 Understanding CMake keywords

- **PUBLIC / PRIVATE / INTERFACE** apply to include directories, compile options, link libraries, etc. They control propagation through the dependency graph.
- **STATIC / SHARED / MODULE** (library types):
  - **STATIC** – archives, linked at compile time.
  - **SHARED** – dynamic libraries (what `build_process_lib` uses).
  - **MODULE** – plugins loaded at runtime (not used here).
- **Generator expressions** (`$<…>`) are evaluated per-configuration/per-target and let us distinguish build vs install interfaces, compiler-specific flags, etc.
- **Install interface** – how consumers “see” the target after installation.
- **Build interface** – how the target behaves when built directly from the source tree.

### 2.5 `find_package()` vs `Config` packages

- When a library installs a `<name>Config.cmake` file (as we do via `install(EXPORT …)`), consumers can call `find_package(name CONFIG)` and CMake will import the target definitions (e.g. `langlawyer::build_process_lib`).
- **Module mode** (`find_package(name)`) searches the CMake module path for `FindName.cmake` scripts. Conan’s `CMakeDeps` generator supplies both module and config variants for third-party packages.
- Because we export a config file, future projects can depend on `LangLawyer` as a package, and Conan knows how to rehydrate the targets from the install tree.

---

## 3. Putting it all together

1. **Environment** – Use `uv sync` in `python_env/` to get Conan installed with the pinned Python version.
2. **Install step** – `conan install` generates `conan_toolchain.cmake` (with the right `ENABLE_SAVE_TEMPS` value) and dependency configs under `build/<profile>/generators/`.
3. **Build step** – `conan build` (or manual `cmake --build`) compiles libraries/executables, putting artefacts in `build/<profile>/bin` and `lib`.
4. **Inspecting intermediate files** – pass `-o langlawyer/*:save_temps=True` (or `-DENABLE_SAVE_TEMPS=ON`) to keep `.ii`, `.s`, and `.bc` outputs.
5. **Packaging** – `conan build` followed by `conan export-pkg . --user=… --channel=…` would package `langlawyer` with the installed headers + exported targets, ready for reuse.

Use this document as a living reference—extend it when you add new options, generators, or CMake patterns.

