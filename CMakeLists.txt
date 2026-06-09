cmake_minimum_required(VERSION 3.16)

project(vehicle_gateway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(Threads REQUIRED)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wpedantic)

# Hardware-independent logic, shared by the gateway and the unit tests.
add_library(gateway_core STATIC
    src/CanDecoder.cpp
)
target_include_directories(gateway_core PUBLIC include)
target_link_libraries(gateway_core PRIVATE project_warnings)
