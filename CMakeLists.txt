# *** CMakeLists.txt
# Root build configuration for the Tiferet C framework.

cmake_minimum_required(VERSION 3.20)
project(tiferet-c
    VERSION 1.0.0
    LANGUAGES C
    DESCRIPTION "Tiferet — A C Framework for Domain-Driven Design"
)

# *** options

option(TIFERET_BUILD_TESTS    "Build unit tests"             ON)
option(TIFERET_BUILD_EXAMPLES "Build example programs"       ON)
option(TIFERET_YAML           "Enable YAML support (libyaml)" OFF)

# *** compiler settings

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# *** library target

# Collect all source files.
file(GLOB_RECURSE TIFERET_SOURCES src/*.c)

add_library(tiferet STATIC ${TIFERET_SOURCES})

target_include_directories(tiferet
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Compiler warnings.
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(tiferet PRIVATE -Wall -Wextra -Wpedantic)
endif()

# *** optional: YAML support

if(TIFERET_YAML)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(YAML REQUIRED yaml-0.1)
    target_link_libraries(tiferet PUBLIC ${YAML_LIBRARIES})
    target_include_directories(tiferet PUBLIC ${YAML_INCLUDE_DIRS})
    target_compile_definitions(tiferet PUBLIC TIFERET_HAS_YAML=1)
endif()

# *** tests

if(TIFERET_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# *** examples

if(TIFERET_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
