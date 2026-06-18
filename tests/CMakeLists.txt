include(FetchContent)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(gateway_tests
    CanDecoderTest.cpp
    ConfigTest.cpp
    SignalJsonTest.cpp
    ThreadSafeQueueTest.cpp
)
target_link_libraries(gateway_tests
    PRIVATE
        gateway_core
        GTest::gtest_main
        Threads::Threads
        project_warnings
)

include(GoogleTest)
gtest_discover_tests(gateway_tests)
