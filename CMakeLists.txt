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
# Set up global inclusions and exclusions for source file quality assurance
# tools. This is intended to filter in external directories (e.g. out-of-tree
# modules) and filter out third-party directories.
#

list(APPEND glob_includes "${SCP_SOURCE_DIR}")

list(APPEND glob_excludes "^${SCP_SOURCE_DIR}/contrib/cmsis/git")

list(APPEND cmake_globs "CMakeLists.txt")
list(APPEND cmake_globs "*.cmake")

list(APPEND markdown_globs "*.md")

#
# Glob sources and place them in their respective variables. Globs for source
# types can be with specified with a `${type}_globs` variable, which should be a
# list of globs to be fed into `file(GLOB_RECURSE ...)`.
#

# cmake-lint: disable=C0103

foreach(type cmake markdown)
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
# Create the final check targets. These targets consist of miscellaneous quality
# assurance tasks like linting and formatting, and act as dummy targets that
# invoke the various tools we set up above.
#

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
