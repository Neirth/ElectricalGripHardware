# Copyright (c) 2020 Wind River Systems, Inc. All Rights Reserved.
#
cmake_minimum_required(VERSION 3.15)
project(ElectricalGripHardware LANGUAGES C CXX)

find_package(libxml2)

foreach(DIR ${CMAKE_INCLUDE_PATH})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${DIR}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${DIR}")
endforeach()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I/opt/vxworks-sdk/vxsdk/sysroot/usr/h/public/")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/opt/vxworks-sdk/vxsdk/sysroot/usr/h/public/")

foreach(DIR ${CMAKE_LIBRARY_PATH})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${DIR}")
endforeach()

# Configuración de compilación para modo debug y release
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -ffast-math")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Oz -ffast-math")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Oz -ffast-math")

add_executable(ElectricalGripHardware 
    src/ElectricalGripHardware.c
    src/services/main_service.c
    src/services/data_collector_service.c
    src/services/predict_load_service.c
    src/utils/main_utils.c
)

target_include_directories(ElectricalGripHardware PRIVATE
    ${__VXWORKS_INCLUDE_PATH__}
    
)

target_link_libraries(ElectricalGripHardware PRIVATE
    LibXml2::LibXml2
    curl
    ssl
    crypto
    z
    m
)

if(CMAKE_SYSTEM_NAME STREQUAL "VxWorks")
    set(CMAKE_EXECUTABLE_SUFFIX ".vxe")
endif()