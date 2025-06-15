cmake_minimum_required(VERSION 3.28.3)
project(api-service)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon CONFIG REQUIRED)
find_package(hiredis CONFIG REQUIRED)
find_package(prometheus-cpp CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(RdKafka CONFIG REQUIRED)

add_executable(api-service src/main.cpp src/note_controller.cpp)

target_link_libraries(api-service PRIVATE 
    Drogon::Drogon
    hiredis::hiredis
    RdKafka::rdkafka
    RdKafka::rdkafka++
    prometheus-cpp::core
    prometheus-cpp::pull
    spdlog::spdlog
)