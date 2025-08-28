add_library(Utils INTERFACE)

target_include_directories(Utils INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_BINARY_DIR}/configured_files/include"
)

find_package(Drogon CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(unofficial-argon2 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)

target_link_libraries(Utils INTERFACE 
    JsonCpp::JsonCpp
    Drogon::Drogon
    unofficial::argon2::libargon2
    spdlog::spdlog
    jwt-cpp::jwt-cpp
)

target_compile_features(Utils INTERFACE cxx_std_20)