# Conan 1 vs Conan 2: What Really Changed (and Why It Matters)

Conan 2 is more than a syntactic refresh—it reframes how the tool participates in the build system.
Instead of sprinkling text-based helpers into CMake, the new model provides context-aware toolchains and dependency metadata that CMake consumes natively.

This article contrasts Conan 1 and Conan 2 through that lens and shows why the modern approach feels first-class in CMake-driven workflows.

---

## 🧩 Old World: Conan 1 Generators

In Conan 1, we usually wrote something like this:

```python
generators = "cmake", "cmake_find_package"
```

These generators created helper files such as:

```
conanbuildinfo.cmake
Findfmt.cmake
```

Then in CMake we would do:

```cmake
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
find_package(fmt REQUIRED)
target_link_libraries(app PRIVATE ${CONAN_LIBS})
```

It worked — but it always felt a bit “non-native.”
Conan injected global variables, polluted CMake paths, and didn’t really understand multiple configurations, toolchains, or cross builds.

---

## 🚀 New World: Conan 2 Generators

In Conan 2, those old generators are gone.
They’ve been replaced by two modern tools:

| Tool               | Purpose                                                            | Generates               |
| ------------------ | ------------------------------------------------------------------ | ----------------------- |
| **CMakeDeps**      | Describes *dependencies* (how to find and link them)               | `<pkg>-config.cmake`    |
| **CMakeToolchain** | Describes *the build environment* (compiler, flags, sysroot, etc.) | `conan_toolchain.cmake` |

Note that they’re not external executables — they’re built-in Python modules under `conan.tools.cmake`.

### The new pattern looks like this:

```python
from conan.tools.cmake import CMakeDeps, CMakeToolchain

def generate(self):
    tc = CMakeToolchain(self)
    tc.variables["CMAKE_CXX_STANDARD"] = "20"  # or any other CMake cache variables you want to set
    tc.generate()

    deps = CMakeDeps(self)
    deps.generate()
```

After `conan install`, the Conan-native build command reuses those generated files automatically:

```bash
conan build . --build-folder=build
```

Behind the scenes it runs CMake’s configure + build phases, already wired to the toolchain and dependency metadata.

If you prefer to drive CMake yourself, point it at the generated toolchain file:

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=build/generators/conan_toolchain.cmake
```

A *toolchain file* is an early-executed CMake script that establishes compilers, flags, sysroots, and other cache variables before the project’s `CMakeLists.txt` runs. By letting Conan generate it, the build behaves exactly as if you had authored a native cross/host toolchain file yourself.

CMake automatically sees all dependencies via `find_package(<pkg> CONFIG)` — no more `include(conanbuildinfo.cmake)` hacks.

---

## 🧱 From Static to Dynamic

Conan 1 wrote everything into one static file (`conanbuildinfo.cmake`) that was valid only for a single configuration.
If you wanted Debug vs Release, you had to re-run `conan install`.

Conan 2’s generators are **multi-config aware**.

```
fmtTargets-debug.cmake
fmtTargets-release.cmake
```

Now you can:

```bash
cmake -G "Ninja Multi-Config" -S . -B build
cmake --build build --config Debug
cmake --build build --config Release
```

or simply let Conan drive the same multi-config build:

```bash
conan install . --build=missing --output-folder=build --settings=build_type=Debug
conan install . --build=missing --output-folder=build --settings=build_type=Release
conan build . --build-folder=build --config Debug
conan build . --build-folder=build --config Release
```

Because the generators are multi-config aware, both Debug and Release metadata coexist in a single `build/` directory, and you can switch configurations without reconfiguring from scratch.

One build folder, multiple configurations — it just works.
IDE integration (Visual Studio, CLion, Xcode) also becomes effortless.

---

## ⚙️ Toolchain Awareness

In Conan 1, you had to manually define toolchain settings:

```python
cmake.definitions["CMAKE_C_COMPILER"] = "clang"
cmake.definitions["CMAKE_CXX_FLAGS"] = "-O2 -Wall"
```

In Conan 2, `CMakeToolchain` does it all for you:

```cmake
# conan_toolchain.cmake (auto-generated)
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "aarch64")
set(CMAKE_C_COMPILER "/opt/aarch64-gcc")
set(CMAKE_CXX_STANDARD "20")
```

You just pass `-DCMAKE_TOOLCHAIN_FILE=...` to CMake,
and it builds with the right compiler, flags, and sysroot — exactly as if you’d written a native toolchain file yourself.

That’s why Conan 2 integrates cleanly with IDEs: it speaks CMake’s native language now.

---

## 🧭 Context Awareness (build / host / test)

Another silent revolution is *context separation*.
Conan 1 had no concept of *where* each dependency was used — everything was built the same way.

Conan 2 introduces clear separation:

| Context   | What it means                                        | Declared by     |
| --------- | ---------------------------------------------------- | --------------- |
| **build** | Tools that run on the build machine (e.g., `protoc`) | `tool_requires` |
| **host**  | The target system your app runs on                   | `requires`      |
| **test**  | Dependencies only for testing                        | `test_requires` |

So if you’re cross-compiling from x86 → ARM, Conan 2 automatically builds tools for x86 but links ARM libraries for the target.
No more mixing architectures accidentally.

---

## 🔍 Side-by-Side Comparison

| Feature                      | Conan 1                         | Conan 2                                                   |
| ---------------------------- | ------------------------------- | --------------------------------------------------------- |
| Dependency generator         | `cmake_find_package`            | `CMakeDeps`                                               |
| Environment generator        | `cmake`, `cmake_paths`          | `CMakeToolchain`                                          |
| Multi-config (Debug/Release) | ❌ No                            | ✅ Yes                                                     |
| Cross compilation            | ⚠️ Manual setup                 | ✅ Automatic                                               |
| IDE integration              | ⚠️ Requires hacks               | ✅ Native (Toolchain file)                                 |
| Build/Host contexts          | ❌ Mixed                         | ✅ Explicit                                                |
| Command style                | `include(conanbuildinfo.cmake)` | `-DCMAKE_TOOLCHAIN_FILE=...` + `find_package(... CONFIG)` |

---

## 🔧 Why It Feels Better

With Conan 2, the generated files are things CMake actually *understands*.
You’re no longer working through Conan-specific magic — you’re just using standard CMake mechanisms that Conan auto-generates for you.

That means:

* Better IDE support.
* Easier debugging.
* Portable cross builds.
* One `build/` folder that just works.

---

## 🧠 TL;DR

| Old Way                    | New Way                  |
| -------------------------- | ------------------------ |
| Global variables and hacks | Real CMake config files  |
| Static one-shot build      | Multi-config aware       |
| Manual flags and compilers | Auto-generated toolchain |
| No cross build separation  | Build/host contexts      |
| “Feels external”           | Feels *native* to CMake  |
