cmake_minimum_required(VERSION 3.15)
project(ally_plugin_system VERSION 1.0.0 LANGUAGES C CXX)

# Set C and C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_EXAMPLES "Build example plugins and hosts" ON)
option(BUILD_TESTS "Build unit tests" ON)

# Common paths
set(ALLY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(ALLY_EXAMPLES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples)
set(ALLY_EXAMPLES_C_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples/c_plugin)
set(ALLY_EXAMPLES_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp_plugin)

# Platform-specific settings
if(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

function(ally_enable_msvc_cxx target)
    if(MSVC)
        foreach(src IN LISTS ARGN)
            set_source_files_properties(${src} PROPERTIES LANGUAGE CXX)
        endforeach()
        target_compile_features(${target} PRIVATE cxx_std_20)
        target_compile_options(${target} PRIVATE /wd4996)
    endif()
endfunction()

#
# Ally Core Library
#
add_library(ally src/ally.c)
target_include_directories(ally PUBLIC
    $<BUILD_INTERFACE:${ALLY_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)

ally_enable_msvc_cxx(ally src/ally.c)

# Platform-specific linking
if(UNIX AND NOT APPLE)
    target_link_libraries(ally PRIVATE dl)
endif()

# Set visibility
if(BUILD_SHARED_LIBS)
    target_compile_definitions(ally PRIVATE ALLY_BUILDING_LIBRARY)
    set_target_properties(ally PROPERTIES
        C_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
endif()

#
# Examples
#
if(BUILD_EXAMPLES)
    # C Calculator Plugin
    add_library(calculator_plugin SHARED
        examples/c_plugin/calculator_plugin.c
    )
    target_include_directories(calculator_plugin PRIVATE
        ${ALLY_INCLUDE_DIR}
        ${ALLY_EXAMPLES_C_DIR}
    )
    set_target_properties(calculator_plugin PROPERTIES
        PREFIX ""
        OUTPUT_NAME "calculator_plugin"
    )
    ally_enable_msvc_cxx(calculator_plugin examples/c_plugin/calculator_plugin.c)

    # C++ String Processor Plugin
    add_library(string_processor_plugin SHARED
        examples/cpp_plugin/string_processor_plugin.cpp
    )
    target_include_directories(string_processor_plugin PRIVATE
        ${ALLY_INCLUDE_DIR}
        ${ALLY_EXAMPLES_CPP_DIR}
    )
    set_target_properties(string_processor_plugin PROPERTIES
        PREFIX ""
        OUTPUT_NAME "string_processor_plugin"
    )

    # C Host
    add_executable(c_host examples/hosts/c_host.c)
    target_include_directories(c_host PRIVATE
        ${ALLY_INCLUDE_DIR}
        ${ALLY_EXAMPLES_DIR}
        ${ALLY_EXAMPLES_C_DIR}
    )
    target_link_libraries(c_host PRIVATE ally)
    ally_enable_msvc_cxx(c_host examples/hosts/c_host.c)

    # C++ Host
    add_executable(cpp_host examples/hosts/cpp_host.cpp)
    target_include_directories(cpp_host PRIVATE
        ${ALLY_INCLUDE_DIR}
        ${ALLY_EXAMPLES_DIR}
        ${ALLY_EXAMPLES_CPP_DIR}
    )
    target_link_libraries(cpp_host PRIVATE ally)
endif()

#
# Unit Tests
#
if(BUILD_TESTS AND BUILD_EXAMPLES)
    enable_testing()

    # C API Unit Tests
    add_executable(test_c_api tests/test_c_api.c)
    target_include_directories(test_c_api PRIVATE
        ${ALLY_INCLUDE_DIR}
        ${ALLY_EXAMPLES_DIR}
        ${ALLY_EXAMPLES_C_DIR}
    )
    target_link_libraries(test_c_api PRIVATE ally)
    add_test(NAME test_c_api COMMAND test_c_api)
    ally_enable_msvc_cxx(test_c_api tests/test_c_api.c)

    # C++ API Unit Tests
    add_executable(test_cpp_api tests/test_cpp_api.cpp)
    target_include_directories(test_cpp_api PRIVATE
        ${ALLY_INCLUDE_DIR}
        ${ALLY_EXAMPLES_DIR}
        ${ALLY_EXAMPLES_CPP_DIR}
    )
    target_link_libraries(test_cpp_api PRIVATE ally)
    add_test(NAME test_cpp_api COMMAND test_cpp_api)

    # Multi-Plugin Loading Tests
    add_executable(test_multi_plugin tests/test_multi_plugin.cpp)
    target_include_directories(test_multi_plugin PRIVATE
        ${ALLY_INCLUDE_DIR}
        ${ALLY_EXAMPLES_DIR}
        ${ALLY_EXAMPLES_C_DIR}
        ${ALLY_EXAMPLES_CPP_DIR}
    )
    target_link_libraries(test_multi_plugin PRIVATE ally)
    add_test(NAME test_multi_plugin COMMAND test_multi_plugin)
endif()

#
# Installation
#
include(GNUInstallDirs)

install(TARGETS ally
    EXPORT ally-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/ally
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT ally-targets
    FILE ally-targets.cmake
    NAMESPACE ally::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ally
)

# Generate config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ally-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ally-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ally
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ally-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ally-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ally-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ally
)
