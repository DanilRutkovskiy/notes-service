find_library(LIBRDKAFKA_LIBRARY NAMES rdkafka PATHS "${CMAKE_CURRENT_LIST_DIR}/../../../vcpkg/installed/x64-linux/lib" NO_DEFAULT_PATH)
find_path(LIBRDKAFKA_INCLUDE_DIR NAMES librdkafka/rdkafka.h PATHS "${CMAKE_CURRENT_LIST_DIR}/../../../vcpkg/installed/x64-linux/include" NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(librdkafka DEFAULT_MSG LIBRDKAFKA_LIBRARY LIBRDKAFKA_INCLUDE_DIR)

if(librdkafka_FOUND)
    set(librdkafka_LIBRARIES ${LIBRDKAFKA_LIBRARY})
    set(librdkafka_INCLUDE_DIRS ${LIBRDKAFKA_INCLUDE_DIR})
endif()