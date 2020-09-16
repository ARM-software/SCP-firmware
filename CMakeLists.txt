#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# cmake-lint: disable=C0301

cmake_minimum_required(VERSION 3.18.3)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(
    SCP
    VERSION 2.7.0
    DESCRIPTION "Arm SCP/MCP Software"
    HOMEPAGE_URL
        "https://developer.arm.com/tools-and-software/open-source-software/firmware/scp-firmware"
    LANGUAGES NONE)

#
# Configure the default build type to be "Release". We choose to default to a
# release build as non-developer consumers will generally want a release build,
# whereas developers who need a debug build are generally familiar with how to
# enable it in the build system.
#

if((NOT CMAKE_BUILD_TYPE) AND (NOT CMAKE_CONFIGURATION_TYPES))
    set(CMAKE_BUILD_TYPE
        "Release"
        CACHE STRING "Build type." FORCE)

    set_property(
        CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel"
                                                "RelWithDebInfo")
endif()

#
# Set up global inclusions and exclusions for source file quality assurance
# tools. This is intended to filter in external directories (e.g. out-of-tree
# modules) and filter out third-party directories.
#

list(APPEND glob_includes "${SCP_SOURCE_DIR}")

list(APPEND glob_excludes "^${SCP_SOURCE_DIR}/contrib/cmsis/git")
list(APPEND glob_excludes "^${SCP_SOURCE_DIR}/contrib/run-clang-format/git")

list(APPEND cmake_globs "CMakeLists.txt")
list(APPEND cmake_globs "*.cmake")

list(APPEND markdown_globs "*.md")

list(APPEND yaml_globs "*.yml")
list(APPEND yaml_globs "*.yaml")

list(APPEND c_globs "*.[ch]")
list(APPEND c_globs "*.[ch]pp")
list(APPEND c_globs "*.[ch]xx")

#
# Glob sources and place them in their respective variables. Globs for source
# types can be with specified with a `${type}_globs` variable, which should be a
# list of globs to be fed into `file(GLOB_RECURSE ...)`.
#

# cmake-lint: disable=C0103

foreach(type cmake markdown yaml c)
    unset(sources)

    foreach(include IN LISTS glob_includes)
        foreach(glob IN LISTS ${type}_globs)
            file(GLOB_RECURSE _sources "${include}/${glob}")
            list(APPEND sources ${_sources})
        endforeach()
    endforeach()

    foreach(exclude IN LISTS glob_excludes)
        list(FILTER sources EXCLUDE REGEX "${exclude}")
    endforeach()

    set(${type}_sources ${sources})
endforeach()

#
# Configure cmake-format.
#

find_package(CMakeFormat OPTIONAL_COMPONENTS Format Lint)

if(TARGET CMakeFormat::Format)
    add_custom_target(
        format-cmake
        COMMAND CMakeFormat::Format -i "${cmake_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Formatting CMake sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND format_targets "format-cmake")

    add_custom_target(
        check-cmake
        COMMAND CMakeFormat::Format --check "${cmake_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Checking CMake sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND check_targets "check-cmake")
endif()

if(TARGET CMakeFormat::Lint)
    add_custom_target(
        lint-cmake
        COMMAND CMakeFormat::Lint --suppress-decorations "${cmake_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Linting CMake sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND lint_targets "lint-cmake")
endif()

#
# Configure markdownlint.
#

find_package(Markdownlint)

if(Markdownlint_FOUND)
    add_custom_target(
        lint-markdown
        COMMAND Markdownlint "${markdown_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Running Markdown sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND lint_targets "lint-markdown")
endif()

#
# Configure yaml-format.
#

find_package(Python3 COMPONENTS Interpreter)

if(Python3_Interpreter_FOUND)
    add_custom_target(
        format-yaml
        COMMAND Python3::Interpreter "${SCP_SOURCE_DIR}/tools/yaml-format.py"
                format -i "${yaml_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Formatting YAML sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND format_targets "format-yaml")

    add_custom_target(
        check-yaml
        COMMAND Python3::Interpreter "${SCP_SOURCE_DIR}/tools/yaml-format.py"
                diff --check "${yaml_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Checking YAML sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND check_targets "check-yaml")
endif()

#
# Configure yamllint.
#

find_package(Yamllint)

if(Yamllint_FOUND)
    add_custom_target(
        lint-yaml
        COMMAND Yamllint -s "${yaml_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Linting YAML sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND lint_targets "lint-yaml")
endif()

#
# Configure Clang-Format.
#

find_package(Clang OPTIONAL_COMPONENTS Format FormatGit)

if(Clang_FormatGit_FOUND)
    add_custom_target(
        format-diff-c
        COMMAND Clang::FormatGit HEAD
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Formatting modified C/C++ sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND format_diff_targets "format-diff-c")
endif()

if(Clang_Format_FOUND)
    add_custom_target(
        format-c
        COMMAND Clang::Format -i "${c_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Formatting C/C++ sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND format_targets "format-c")
endif()

#
# Configure run-clang-format.
#

find_package(Python3 COMPONENTS Interpreter)

if(Python3_Interpreter_FOUND)
    add_custom_target(
        check-c
        COMMAND
            Python3::Interpreter
            "${SCP_SOURCE_DIR}/contrib/run-clang-format/git/run-clang-format.py"
            --clang-format-executable "${Clang_Format_EXECUTABLE}"
            "${c_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Checking C sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND check_targets "check-c")
endif()

#
# Create the final check targets. These targets consist of miscellaneous quality
# assurance tasks like linting and formatting, and act as dummy targets that
# invoke the various tools we set up above.
#

add_custom_target(
    format-diff
    DEPENDS ${format_diff_targets}
    COMMENT "Formatting modified sources...")

add_custom_target(
    format
    DEPENDS ${format_targets}
    COMMENT "Formatting all sources...")

add_custom_target(
    lint
    DEPENDS ${lint_targets}
    COMMENT "Linting all sources...")

add_custom_target(
    check
    DEPENDS ${check_targets} lint
    COMMENT "Checking lint...")
