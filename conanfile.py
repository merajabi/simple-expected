from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake
from conan.tools.env import VirtualBuildEnv

class MyPackageConan(ConanFile):
    name = "mypackage"
    version = "1.0"
    settings = "os", "compiler", "build_type", "arch"
    # generators = "CMakeDeps", "CMakeToolchain", "VirtualBuildEnv"

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

