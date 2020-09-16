#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#[=======================================================================[.rst:
FindCMakeFormat
---------------

Finds cmake-format, the suite of quality assurance tools for CMake source files.

Use this module by invoking ``find_package`` with the form::

  find_package(CMakeFormat [REQUIRED] [COMPONENTS <component>...]
    [OPTIONAL_COMPONENTS <component>...])

This module supports the following components:

* ``Format``: Locate ``cmake-format``, the CMake formatter.
* ``Lint``: Locate ``cmake-lint``, the CMake linter.

Imported Targets
^^^^^^^^^^^^^^^^

``CMakeFormat::Format``
  ``cmake-format``. Target defined if component ``Format`` is found.

``CMakeFormat::Lint``
  ``cmake-lint``. Target defined if component ``Lint`` is found.

Result Variables
^^^^^^^^^^^^^^^^

``CMakeFormat_FOUND``
  If false, none of the requested components were found.

``CMakeFormat_Format_FOUND``
  If false, ``cmake-format`` was not found.

``CMakeFormat_Format_EXECUTABLE``
  The full path to ``cmake-format``.

``CMakeFormat_Lint_FOUND``
  If false, ``cmake-lint`` was not found.

``CMakeFormat_Lint_EXECUTABLE``
  The full path to ``cmake-lint``.
#]=======================================================================]

include_guard()

# cmake-lint: disable=C0103

find_program(CMakeFormat_Format_EXECUTABLE "cmake-format")
find_program(CMakeFormat_Lint_EXECUTABLE "cmake-lint")

if(CMakeFormat_Format_EXECUTABLE)
    set(CMakeFormat_Format_FOUND TRUE)

    mark_as_advanced(CMakeFormat_Format_EXECUTABLE)

    add_executable(CMakeFormat::Format IMPORTED)

    set_target_properties(
        CMakeFormat::Format
        PROPERTIES IMPORTED_LOCATION "${CMakeFormat_Format_EXECUTABLE}")
endif()

if(CMakeFormat_Lint_EXECUTABLE)
    set(CMakeFormat_Lint_FOUND TRUE)

    mark_as_advanced(CMakeFormat_Lint_EXECUTABLE)

    add_executable(CMakeFormat::Lint IMPORTED)

    set_target_properties(
        CMakeFormat::Lint
        PROPERTIES IMPORTED_LOCATION "${CMakeFormat_Lint_EXECUTABLE}")
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(CMakeFormat HANDLE_COMPONENTS)
