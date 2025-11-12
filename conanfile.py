from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout


class LangLawyerRecipe(ConanFile):
    name = "langlawyer"
    version = "0.1.0"

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    exports_sources = (
        "CMakeLists.txt",
        "auto/*",
        "build_process/*",
    )

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        pass

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

