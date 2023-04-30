import os

from conan import ConanFile


class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def imports(self):
        root_dir = os.path.dirname(os.path.abspath(__file__))
        self.copy("*", src="include", dst=os.path.join(root_dir, "deps/include"))
        self.copy("*", src="lib", dst=os.path.join(root_dir, "deps/lib"))

    def requirements(self):
        self.requires("gtest/1.13.0")
