from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout, CMakeDeps
from conan.errors import ConanInvalidConfiguration
import os

class ElectricalGripHardwareConan(ConanFile):
    name = "ElectricalGripHardware"
    version = "1.0"
    license = "Tu_Licencia_Aquí"
    author = "Tu Nombre <tu.email@dominio.com>"
    url = "https://github.com/tu_usuario/ElectricalGripHardware"
    description = "Descripción de tu proyecto"
    topics = ("tensorflow-lite", "vxworks", "curl", "onnxruntime")
    settings = "os", "compiler", "build_type", "arch"
    options = { "shared": [True, False], "fPIC": [True, False] }
    default_options = {
        "shared": False,  # Cambiar a False para linkado estático
        "fPIC": False,
        "*:shared": False,  # Forzar estático para todas las deps
    }

    exports_sources = "src/*", "CMakeLists.txt", "include/*", "vxworks_toolchain.cmake"

    def layout(self):
        cmake_layout(self)

    def validate(self):
        if self.settings.os != "VxWorks":
            raise ConanInvalidConfiguration("Este paquete solo es compatible con VxWorks.")

        if self.options.shared:
            raise ConanInvalidConfiguration("No se permiten shared libraries en VxWorks")

    def configure(self):
        # Opciones específicas para abseil
        # Asegúrate de que estas opciones están definidas en la receta de abseil
        self.options["abseil/*"].cxx_standard = "20"  # Cambiar a C++20
        self.options["abseil/*"].use_system_compiler = True

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

        # Incluir Threads en CMake
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
        self.cpp_info.libs = ["ElectricalGripHardware"]  # Ajusta según el nombre real de la biblioteca

    # Dependencias con override
    def requirements(self):
        self.requires("libxml2/2.13.4")
