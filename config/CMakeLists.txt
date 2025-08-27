configure_file(
    "config.hpp.in"
    "${CMAKE_BINARY_DIR}/configured_files/include/config.hpp"
    ESCAPE_QUOTES
)

configure_file(
    "note-service-drogon-db-config.json.in"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/note-service-drogon-db-config.json"
    @ONLY
)

configure_file(
    "auth-service-drogon-db-config.json.in"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/auth-service-drogon-db-config.json"
    @ONLY
)