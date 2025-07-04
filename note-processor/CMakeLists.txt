cmake_minimum_required(VERSION 3.28.3)
project(note-processor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(RdKafka CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_package(clickhouse REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(cityhash CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)

add_executable(note-processor src/main.cpp)

target_include_directories(note-processor PRIVATE 
                           ${clickhouse_INCLUDE_DIRS})

target_link_libraries(note-processor PRIVATE
    RdKafka::rdkafka
    RdKafka::rdkafka++
    libpqxx::pqxx
    ${clickhouse_LIBS}
    spdlog::spdlog
    cityhash
    JsonCpp::JsonCpp
)