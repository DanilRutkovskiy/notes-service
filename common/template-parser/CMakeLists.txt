add_library(TemplateParser INTERFACE)

target_include_directories(TemplateParser INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_BINARY_DIR}/configured_files/include"
)

find_package(jsoncpp CONFIG REQUIRED)
find_package(boost_mp11 CONFIG REQUIRED)
find_package(boost_describe CONFIG REQUIRED)

target_link_libraries(TemplateParser INTERFACE 
    JsonCpp::JsonCpp
    Boost::mp11
    Boost::describe
)

target_compile_features(TemplateParser INTERFACE cxx_std_20)