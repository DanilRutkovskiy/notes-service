configure_file(
    "config.hpp.in"
    "${CMAKE_BINARY_DIR}/configured_files/include/config.hpp"
    ESCAPE_QUOTES
)

configure_file(
    "drogon-config.json.in"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/drogon-config.json"
    @ONLY
)
