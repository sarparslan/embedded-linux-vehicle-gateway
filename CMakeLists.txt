cmake_minimum_required(VERSION 3.16)

project(vehicle_gateway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(Threads REQUIRED)
find_package(nlohmann_json 3.9.0 REQUIRED)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wpedantic)

# Hardware-independent logic, shared by the gateway and the unit tests.
add_library(gateway_core STATIC
    src/CanDecoder.cpp
    src/Config.cpp
    src/SignalJson.cpp
)
target_include_directories(gateway_core PUBLIC include)
target_link_libraries(gateway_core
    PUBLIC nlohmann_json::nlohmann_json
    PRIVATE project_warnings
)

add_executable(vehicle_gateway
    src/main.cpp
    src/CanReader.cpp
    src/Logger.cpp
    src/MqttPublisher.cpp
)
target_link_libraries(vehicle_gateway
    PRIVATE
        gateway_core
        mosquitto
        Threads::Threads
        project_warnings
)

option(VEHICLE_GATEWAY_BUILD_TESTS "Build the unit tests (downloads GoogleTest)" OFF)

if(VEHICLE_GATEWAY_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
