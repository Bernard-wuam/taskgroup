project(TaskGroup VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(TaskGroup INTERFACE)
add_library(TaskGroup::TaskGroup ALIAS TaskGroup)

target_sources(
    TaskGroup
    INTERFACE
    FILE_SET HEADERS TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include
    FILES ${CMAKE_CURRENT_LIST_DIR}/include/taskgroup/taskgroup.h
)

target_link_libraries(TaskGroup INTERFACE Boost::beast OpenSSL::SSL OpenSSL::Crypto)

install(TARGETS TaskGroup
    EXPORT TaskGroupTargets
    FILE_SET HEADERS)

install(EXPORT TaskGroupTargets
    DESTINATION share/cmake/taskgroup
    NAMESPACE TaskGroup::)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/taskgroupConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/TaskGroupConfig.cmake
    INSTALL_DESTINATION share/cmake/taskgroup
    PATH_VARS CMAKE_INSTALL_PREFIX)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/TaskGroupConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/TaskGroupConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/TaskGroupConfigVersion.cmake
    DESTINATION share/cmake/taskgroup
)