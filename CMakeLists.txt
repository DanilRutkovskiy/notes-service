cmake_minimum_required(VERSION 3.30)

project(
    note-service
    VERSION 1.0.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(USE_VCPKG "Whether to use VCPKG" ON)

#CONFIG
    ##REDIS
    set(REDIS_PORT "6379")
    set(REDIS_HOST "127.0.0.1")
    ##KAFKA
    set(KAFKA_PORT "9092")
    set(KAFKA_HOST "127.0.0.1")
    #JWT-secret-key
    set(JWT_SECRET_KEY "jX7Bgw+Fe1L9zLllVtblTjhiQXKgL0cPDebVziklnrZzm5Rct/oBMM4OAipw1i6vsKkBixD9LH/PchuHzoo6TQ==")
    ##note-service
    set(NOTE_SERVICE_HOST "0.0.0.0")
    set(NOTE_SERVICE_PORT "8080")
    set(NOTE_SERVICE_DB_HOST "0.0.0.0")
    set(NOTE_SERVICE_DB_PORT "60001")
    set(NOTE_SERVICE_DB_NAME "notes-db")
    set(NOTE_SERVICE_DB_USER "user")
    set(NOTE_SERVICE_DB_PASSWORD "password")
    ##auth-service
    set(AUTH_SERVICE_HOST "0.0.0.0")
    set(AUTH_SERVICE_PORT "8081")
    set(AUTH_SERVICE_DB_HOST "0.0.0.0")
    set(AUTH_SERVICE_DB_PORT "60002")
    set(AUTH_SERVICE_DB_NAME "auth-db")
    set(AUTH_SERVICE_DB_USER "user")
    set(AUTH_SERVICE_DB_PASSWORD "password")

if(USE_VCPKG)
    message(STATUS "Using VCPKG")
    include(${CMAKE_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake)
endif()

add_subdirectory(note-service)
add_subdirectory(auth-service)
add_subdirectory(config)
add_subdirectory(common)