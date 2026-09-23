cmake_minimum_required(VERSION 3.15)
project(matching_engine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(include)

# Core engine as a static library -- linked directly into the Drogon backend
# (see ../backend/CMakeLists.txt) as well as the demo/tests/benchmark below.
add_library(engine_core STATIC src/OrderBook.cpp)

add_executable(demo src/main.cpp)
target_link_libraries(demo PRIVATE engine_core)

add_executable(run_tests tests/test_orderbook.cpp)
target_link_libraries(run_tests PRIVATE engine_core)

add_executable(benchmark benchmark/benchmark.cpp)
target_link_libraries(benchmark PRIVATE engine_core)
