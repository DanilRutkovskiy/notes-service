cmake_minimum_required(VERSION 3.30)

project(
    note-service
    VERSION 1.0.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(USE_VCPKG "Whether to use VCPKG" ON)

#CONFIG
    ##REDIS
    set(REDIS_PORT "6379")
    set(REDIS_HOST "127.0.0.1")
    ##KAFKA
    set(KAFKA_PORT "9092")
    set(KAFKA_HOST "127.0.0.1")
    ##api-service
    set(API_SERVICE_HOST "0.0.0.0")
    set(API_SERVICE_PORT "8080")

if(USE_VCPKG)
    message(STATUS "Using VCPKG")
    include(${CMAKE_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake)
endif()

add_subdirectory(api-service)
add_subdirectory(note-processor)
add_subdirectory(config)
add_subdirectory(common)