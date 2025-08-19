cmake_minimum_required(VERSION 3.30)
project(note-processor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(RdKafka CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)

add_executable(note-processor 
    "src/main.cpp"
)

target_include_directories(note-processor PRIVATE 
    "include"
    "${CMAKE_BINARY_DIR}/configured_files/include"
)

target_link_libraries(note-processor PRIVATE
    RdKafka::rdkafka
    RdKafka::rdkafka++
    libpqxx::pqxx
    spdlog::spdlog
    JsonCpp::JsonCpp
    TemplateParser
    pqxx
)