cmake_minimum_required(VERSION 3.28.3)
project(note-processor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(librdkafka CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_package(clickhouse-cpp CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable(note-processor src/main.cpp)

target_link_libraries(note-processor PRIVATE
    RdKafka::rdkafka
    pqxx
    clickhouse
    spdlog::spdlog
)