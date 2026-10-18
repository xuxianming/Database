cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Database VERSION 1.0.0 LANGUAGES CXX)

option(DB_BUILD_TOOL "Build the my_tool demo" ON)
option(DB_BUILD_TESTS "Build the GoogleTest test suite" OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

file(GLOB_RECURSE DATABASE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/BasePlatfrom/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Common/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/DataModel/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Utils/*.cpp"
)

add_library(database SHARED ${DATABASE_SOURCES})
add_library(Database::database ALIAS database)
target_include_directories(database PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(database PUBLIC cxx_std_20)
set_target_properties(database PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

install(TARGETS database
    EXPORT DatabaseTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT DatabaseTargets
    FILE DatabaseTargets.cmake
    NAMESPACE Database::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Database
)

configure_package_config_file(
    cmake/DatabaseConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/DatabaseConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Database
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/DatabaseConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/DatabaseConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/DatabaseConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Database
)

if(DB_BUILD_TOOL)
    find_package(slang REQUIRED)
    add_executable(my_tool src/main.cc)
    target_link_libraries(my_tool PRIVATE slang::slang)
endif()

if(DB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
