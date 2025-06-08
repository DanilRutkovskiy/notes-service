set(VCPKG_ROOT "../../../../vcpkg/installed/x64-linux")

find_library(LIBRDKAFKA_LIBRARY NAMES rdkafka PATHS "${CMAKE_CURRENT_LIST_DIR}/${VCPKG_ROOT}/lib" NO_DEFAULT_PATH)
if(NOT LIBRDKAFKA_LIBRARY)
  message(FATAL_ERROR "Could not find rdkafka library")
endif()

find_library(LIBRDKAFKACPP_LIBRARY NAMES rdkafka++ PATHS "${CMAKE_CURRENT_LIST_DIR}/${VCPKG_ROOT}/lib" NO_DEFAULT_PATH)
if(NOT LIBRDKAFKACPP_LIBRARY)
  message(FATAL_ERROR "Could not find rdkafka++ library")
endif()

find_library(LZ4_LIBRARY NAMES lz4 PATHS "${CMAKE_CURRENT_LIST_DIR}/${VCPKG_ROOT}/lib" NO_DEFAULT_PATH)
if(NOT LZ4_LIBRARY)
  message(FATAL_ERROR "Could not find lz4 library")
endif()

find_path(LIBRDKAFKA_INCLUDE_DIR NAMES librdkafka/rdkafka.h PATHS "${CMAKE_CURRENT_LIST_DIR}/${VCPKG_ROOT}/include" NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(librdkafka DEFAULT_MSG LIBRDKAFKA_LIBRARY LIBRDKAFKA_INCLUDE_DIR)

if(librdkafka_FOUND)
    set(librdkafka_LIBS ${LIBRDKAFKACPP_LIBRARY} ${LIBRDKAFKA_LIBRARY} ${LZ4_LIBRARY})
    set(librdkafka_INCLUDE_DIRS ${LIBRDKAFKA_INCLUDE_DIR})
endif()