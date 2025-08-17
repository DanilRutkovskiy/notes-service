cmake_minimum_required(VERSION 3.30)
project(api-service VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon CONFIG REQUIRED)

set(API_SERVICE_SOURCE
    "src/main.cpp" 
    "src/note_controller.cpp"
)

add_executable(api-service ${API_SERVICE_SOURCE})

target_include_directories(api-service PRIVATE include)

target_link_libraries(api-service PRIVATE 
    Drogon::Drogon
    #hiredis::hiredis
    #RdKafka::rdkafka
    #RdKafka::rdkafka++
    #prometheus-cpp::core
    #prometheus-cpp::pull
    #spdlog::spdlog
)