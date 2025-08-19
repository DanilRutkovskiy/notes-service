cmake_minimum_required(VERSION 3.30)
project(note-service VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(hiredis CONFIG REQUIRED)
find_package(RdKafka CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)

set(API_SERVICE_SOURCE
    "src/main.cpp" 
    "src/note_controller.cpp"
)

add_executable(note-service ${API_SERVICE_SOURCE})

target_precompile_headers(note-service PRIVATE
    "src/pch.hpp"
)

target_include_directories(note-service PRIVATE 
    "include"
    "${CMAKE_BINARY_DIR}/configured_files/include"
)

target_link_libraries(note-service PRIVATE 
    Drogon::Drogon
    hiredis::hiredis
    RdKafka::rdkafka
    RdKafka::rdkafka++
    JsonCpp::JsonCpp
    spdlog::spdlog
    TemplateParser
    pqxx
    #prometheus-cpp::core
    #prometheus-cpp::pull
)