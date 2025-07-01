from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout
from conan.tools.env import VirtualBuildEnv
from conan.tools.files import copy
import os

class MyPackageConan(ConanFile):
    name = "simple_expected"
    version = "1.0"
    settings = "os", "compiler", "build_type", "arch"
    # generators = "CMakeDeps", "CMakeToolchain", "VirtualBuildEnv"
    exports_sources = "include/*", "CMakeLists.txt"
    no_copy_source = True  # For header-only

    def layout(self):
        cmake_layout(self)

    def package_id(self):
        self.info.clear()  # Header-only has no binaries

    # 1. Declare CMake as tool_requires
    def build_requirements(self):
        self.tool_requires("cmake/[>=3.15]") #"ninja/1.11.1"
    
    # 4. Regular package methods
    def requirements(self):
        self.test_requires("gtest/[>=1.11.0]")  # Example test dependency
    
    def generate(self):
        # Ensure build environment is fully set up
        env = VirtualBuildEnv(self)
        env.generate(scope="build")

        # Explicitly generate environment scripts
        tc = CMakeToolchain(self)
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self):
        cmake = CMake(self)
        cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        # For header-only, also explicitly copy headers
        copy(self, "*.hpp", 
            src=os.path.join(self.source_folder, "include"),
            dst=os.path.join(self.package_folder, "include"),
            keep_path=True)  # This maintains the mr/ subdirectory structure

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "SimpleExpected")
        self.cpp_info.set_property("cmake_target_name", "SimpleExpected::SimpleExpected")
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []

