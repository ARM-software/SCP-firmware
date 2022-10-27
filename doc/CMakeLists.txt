#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# cmake-lint: disable=C0301

find_package(Doxygen OPTIONAL_COMPONENTS dot mscgen dia)

if(NOT DOXYGEN_FOUND OR DOXYGEN_VERSION VERSION_LESS 1.8.13)
    return()
endif()
#
# This fairly monstrous block of code actually does a deceptively simple
# operation: it collects all the sources, include directories and definitions
# given to each target that we are building, and surrounds them in quotes.
#
# The complexity comes in the fact that we do not know all of these until
# generation time, at which point it is much harder to do even simple string
# manipulation on them. Still, it can be done, and this allows us to mirror (to
# the extent that we can) precisely what has been compiled and how to Doxygen.
#

# cmake-format: off

set(scp_sources "\"$<JOIN:${scp_sources},\" \">\"")
set(scp_includes "\"$<JOIN:${scp_includes},\" \">\"")
set(scp_defines $<JOIN:${scp_defines}, >)

list(APPEND scp_sources "${SCP_SOURCE_DIR}/change_log.md")
list(APPEND scp_sources "${SCP_SOURCE_DIR}/license.md")
list(APPEND scp_sources "${SCP_SOURCE_DIR}/readme.md")
list(APPEND scp_sources "${SCP_SOURCE_DIR}/user_guide.md")

list(APPEND scp_sources "$(CMAKE_CURRENT_SOURCE_DIR)/architecture_support.md")
list(APPEND scp_sources "$(CMAKE_CURRENT_SOURCE_DIR)/cmsis.md")
list(APPEND scp_sources "$(CMAKE_CURRENT_SOURCE_DIR)/code_rules.md")
list(APPEND scp_sources "$(CMAKE_CURRENT_SOURCE_DIR)/code_style.md")
list(APPEND scp_sources "$(CMAKE_CURRENT_SOURCE_DIR)/deferred_response_architecture.md")
list(APPEND scp_sources "$(CMAKE_CURRENT_SOURCE_DIR)/framework.md")
list(APPEND scp_sources "$(CMAKE_CURRENT_SOURCE_DIR)/glossary.md")
list(APPEND scp_sources "${CMAKE_CURRENT_SOURCE_DIR}/build_system.md")
list(APPEND scp_sources "${CMAKE_CURRENT_SOURCE_DIR}/cmake_readme.md")

# cmake-format: on

#
# Configure the Doxyfile.
#

# cmake-lint: disable=C0103
# cmake-format: off

set(DOXYGEN_PREDEFINED "${scp_defines}")

set(DOXYGEN_PROJECT_LOGO "${CMAKE_CURRENT_SOURCE_DIR}/media/arm-logo-blue-rgb.svg")

list(APPEND DOXYGEN_EXCLUDE_PATTERNS "${SCP_SOURCE_DIR}/contrib/*")

list(APPEND DOXYGEN_STRIP_FROM_PATH "${SCP_SOURCE_DIR}")
list(APPEND DOXYGEN_STRIP_FROM_PATH "${SCP_BINARY_DIR}")

# cmake-format: on

set(DOXYGEN_BUILTIN_STL_SUPPORT "YES")
set(DOXYGEN_EXTRACT_ALL "YES")
set(DOXYGEN_GENERATE_TREEVIEW "YES")
set(DOXYGEN_MACRO_EXPANSION "NO")
set(DOXYGEN_SOURCE_BROWSER "YES")

doxygen_add_docs(doc "${scp_sources}" "${scp_includes}")

#
# The built-in Doxygen support creates a file at configure time that expands
# `${VARIABLES}` but, because it doesn't run at generate time, cannot expand
# `$<GENERATOR_EXPRESSIONS>`. We need to regenerate the Doxyfile and forcibly
# expand these generator expressesions in order to make it valid.
#

file(
    GENERATE
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.doc"
    INPUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.doc")
