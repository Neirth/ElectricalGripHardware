# Copyright (c) 2020 Wind River Systems, Inc. All Rights Reserved.
#
cmake_minimum_required(VERSION 3.15)
project(ElectricalGripHardware LANGUAGES C CXX)

# Configurar C++ standard y VxWorks
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER "wr-c++")

# Forzar la inclusión de rutas en flags del compilador
foreach(DIR ${CMAKE_INCLUDE_PATH})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${DIR}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${DIR}")
endforeach()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lcurl -lm")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lcurl -lm")

foreach(DIR ${CMAKE_LIBRARY_PATH})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${DIR}")
endforeach()

find_package(libxml2 REQUIRED)

# Incluir el toolchain de Conan
include(${CMAKE_TOOLCHAIN_FILE})
set(CURL_LIBRARIES "/opt/vxworks-sdk/vxsdk/sysroot/usr/lib/common/libcurl.a")


# Añadir el ejecutable
add_executable(ElectricalGripHardware 
    src/ElectricalGripHardware.c
    src/services/main_service.c
    src/services/data_collector_service.c
    src/services/predict_load_service.c
    src/utils/main_utils.c
)

# Enlazar las bibliotecas necesarias correctamente
target_link_libraries(ElectricalGripHardware PRIVATE
    LibXml2::LibXml2 
    ${CURL_LIBRARIES}
    ssl
    crypto
    z
    m
)

# Configurar el sufijo del ejecutable para VxWorks
if(CMAKE_SYSTEM_NAME STREQUAL "VxWorks")
    set(CMAKE_EXECUTABLE_SUFFIX ".vxe")
endif()

# Otras configuraciones...
