add_library(Utils INTERFACE)

target_include_directories(Utils INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_BINARY_DIR}/configured_files/include"
)

find_package(Drogon CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(unofficial-argon2 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(TemplateParser INTERFACE 
    JsonCpp::JsonCpp
    Drogon::Drogon
    unofficial::argon2::libargon2
    spdlog::spdlog
)

target_compile_features(TemplateParser INTERFACE cxx_std_20)