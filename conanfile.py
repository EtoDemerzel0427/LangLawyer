import subprocess

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout


class LangLawyerRecipe(ConanFile):
    name = "langlawyer"
    package_type = "application"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "save_temps": [True, False],
    }
    default_options = {
        "save_temps": False,
    }
    exports_sources = "*"  # export all files in the root directory except for those in .conanignore

    def set_version(self):
        try:
            tag = subprocess.check_output(
                ["git", "describe", "--tags", "--dirty", "--always"],
                cwd=self.recipe_folder,
                stderr=subprocess.DEVNULL,
            ).decode().strip()
            if tag:
                self.version = tag
                return
        except Exception:  # noqa: BLE001
            self.output.warn("Failed to get version from git")

        self.version = "0.0.0"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["ENABLE_SAVE_TEMPS"] = "ON" if self.options.save_temps else "OFF"
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def requirements(self):
        pass

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

