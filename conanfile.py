from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout, CMakeDeps
from conan.errors import ConanInvalidConfiguration
import os

class ElectricalGripHardwareConan(ConanFile):
    name = "ElectricalGripHardware"
    version = "1.0"
    license = "MIT"
    author = "Sergio Martinez <7413024+Neirth@users.noreply.github.com>"
    url = "https://github.com/Neirth/ElectricalGripHardware/"
    description = "A try to made a Real Time Application inspired in the bases of my Bachellor's Thesis. Spike of Commercial usages..."
    topics = ("tensorflow-lite", "vxworks", "curl", "onnxruntime")
    settings = "os", "compiler", "build_type", "arch"
    options = { "shared": [True, False], "fPIC": [True, False] }
    default_options = {
        "shared": False,
        "fPIC": False,
        "*:shared": False,
    }

    exports_sources = "src/*", "CMakeLists.txt", "include/*", "vxworks_toolchain.cmake"

    def layout(self):
        cmake_layout(self)

    def validate(self):
        if self.settings.os != "VxWorks":
            raise ConanInvalidConfiguration("This rtp only works with VxWorks")

        if self.options.shared:
            raise ConanInvalidConfiguration("This rtp only works with static libraries")

    def build(self):
        cmake = CMake(self)
        isConfigured = os.path.exists("CMakeCache.txt")
        if not isConfigured:
            cmake.configure()
        cmake.build()

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["CMAKE_CXX_STANDARD"] = "20"
        tc.variables["CMAKE_CXX_STANDARD_REQUIRED"] = "ON"
        tc.variables["BUILD_TESTING"] = "OFF"

        tc.variables["CMAKE_THREAD_LIBS_INIT"] = "-lpthread"
        tc.variables["CMAKE_HAVE_THREADS_LIBRARY"] = "TRUE"
        tc.variables["CMAKE_USE_PTHREADS_INIT"] = "TRUE"
    
        tc.generate()
        
        cmake = CMakeDeps(self)
        cmake.generate()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["ElectricalGripHardware"]

    def requirements(self):
        self.requires("libxml2/2.13.4")
