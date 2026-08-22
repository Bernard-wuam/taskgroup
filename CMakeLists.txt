add_library(TaskGroup INTERFACE)
add_library(TaskGroup::TaskGroup ALIAS TaskGroup)

target_sources(
    TaskGroup
    INTERFACE
    FILE_SET HEADERS TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include
    FILES ${CMAKE_CURRENT_LIST_DIR}/include/taskgroup/taskgroup.h
)

find_package(boost_beast CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

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