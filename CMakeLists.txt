cmake_minimum_required(VERSION 3.0)
project(bindb)

set(CMAKE_CXX_STANDARD 17)
add_library(bindb bindb.h)
set_target_properties(bindb PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(bindb PROPERTIES PUBLIC_HEADER "bindb.h")

option(TESTS "Compile tests" ON)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-result -Wno-sign-compare")

if (TESTS)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()

    add_executable(bindb_tests tests.cpp)
    add_executable(bindb_benchmarks benchmarks.cpp)

    target_compile_definitions(bindb_tests PRIVATE -DTESTS)
    target_compile_definitions(bindb_benchmarks PRIVATE -DTESTS)

    target_link_libraries(bindb_tests gtest_main)
    target_link_libraries(bindb_benchmarks gtest_main)

    include(GoogleTest)
    gtest_discover_tests(bindb_tests)
endif ()

install(TARGETS bindb
        LIBRARY DESTINATION "lib"
        PUBLIC_HEADER DESTINATION "include")
