set(VCPKG_ROOT "../../../../vcpkg/installed/x64-linux")

find_library(CLICKHOUSE_LIBRARY 
             NAMES clickhouse-cpp-lib clickhouse-cpp clickhouse
             PATHS "${CMAKE_CURRENT_LIST_DIR}/${VCPKG_ROOT}/lib" NO_DEFAULT_PATH)

if(NOT CLICKHOUSE_LIBRARY)
  message(FATAL_ERROR "Could not find clickhouse  library")
endif()

find_path(CLICKHOUSE_INCLUDE_DIR
          NAMES clickhouse/client.h 
          PATHS "${CMAKE_CURRENT_LIST_DIR}/${VCPKG_ROOT}/include" NO_DEFAULT_PATH)

if(NOT CLICKHOUSE_INCLUDE_DIR)
  message(FATAL_ERROR "Could not find clickhouse include directory")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(clickhouse DEFAULT_MSG CLICKHOUSE_LIBRARY CLICKHOUSE_INCLUDE_DIR)

if(clickhouse_FOUND)
    set(clickhouse_LIBS ${CLICKHOUSE_LIBRARY})
    set(clickhouse_INCLUDE_DIRS ${CLICKHOUSE_INCLUDE_DIR})
endif()