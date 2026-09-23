cmake_minimum_required(VERSION 3.15)
project(order_matching_backend CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

find_package(Drogon REQUIRED)

add_executable(order_matching_backend
    main.cc
    controllers/HealthController.cc
    controllers/OrdersController.cc
    controllers/OrderBookController.cc
    controllers/TradesController.cc
    ws/MarketDataWs.cc
    services/BookManager.cc
    services/Persistence.cc
    services/RedisPublisher.cc
    # Compiled directly rather than linked as a separate CMake target to
    # avoid target-name collisions with ../engine's own CMakeLists.txt
    # (which builds the same file into its own demo/tests/benchmark).
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/src/OrderBook.cpp
)

target_include_directories(order_matching_backend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/include
)

target_link_libraries(order_matching_backend PRIVATE Drogon::Drogon)

# Copy config.json next to the built binary so `./order_matching_backend`
# picks it up automatically when run from the build directory.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.json
                ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)
