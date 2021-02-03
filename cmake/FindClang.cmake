#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include_guard()

#[=======================================================================[.rst:
FindClangFormat
---------------

Finds tools and libraries belonging to Clang.

Use this module by invoking ``find_package`` with the form::

  find_package(Clang [REQUIRED] [COMPONENTS <component>...]
    [OPTIONAL_COMPONENTS <component>...])

This module supports the following components:

* ``Format``: Locate ``clang-format``.
* ``FormatGit``: Locate ``git-clang-format``, the Git integration of
  ``clang-formatt`.

Imported Targets
^^^^^^^^^^^^^^^^

``Clang::Format``
  ``clang-format``. Target defined if component ``Format`` is found.

``Clang::FormatGit``
  ``git-clang-format``. Target defined if component ``FormatGit`` is found.

Result Variables
^^^^^^^^^^^^^^^^

``Clang_FOUND``
  If false, none of the requested components were found.

``Clang_Format_FOUND``
  If false, ``clang-format`` was not found.

``Clang_Format_EXECUTABLE``
  The full path to ``clang-format``.

``Clang_FormatGit_FOUND``
  If false, ``git-clang-format`` was not found.

``Clang_FormatGit_EXECUTABLE``
  The full path to ``git-clang-format``.
#]=======================================================================]

# cmake-lint: disable=C0103

find_program(
    Clang_Format_EXECUTABLE
    NAMES "clang-format-11"
          "clang-format-10"
          "clang-format-9"
          "clang-format-8"
          "clang-format-7"
          "clang-format-6.0"
          "clang-format-5.0"
          "clang-format-4.0"
          "clang-format-3.9"
          "clang-format-3.8"
          "clang-format-3.7"
          "clang-format-3.6"
          "clang-format")

find_program(
    Clang_FormatGit_EXECUTABLE
    NAMES "git-clang-format-11"
          "git-clang-format-10"
          "git-clang-format-9"
          "git-clang-format-8"
          "git-clang-format-7"
          "git-clang-format-6.0"
          "git-clang-format-5.0"
          "git-clang-format-4.0"
          "git-clang-format-3.9"
          "git-clang-format-3.8"
          "git-clang-format-3.7"
          "git-clang-format-3.6"
          "git-clang-format")

if(Clang_Format_EXECUTABLE)
    set(Clang_Format_FOUND TRUE)

    mark_as_advanced(Clang_Format_EXECUTABLE)

    add_executable(Clang::Format IMPORTED)

    set_target_properties(
        Clang::Format PROPERTIES IMPORTED_LOCATION "${Clang_Format_EXECUTABLE}")
endif()

if(Clang_FormatGit_EXECUTABLE)
    set(Clang_FormatGit_FOUND TRUE)

    mark_as_advanced(Clang_FormatGit_EXECUTABLE)

    add_executable(Clang::FormatGit IMPORTED)

    set_target_properties(
        Clang::FormatGit
        PROPERTIES IMPORTED_LOCATION "${Clang_FormatGit_EXECUTABLE}")
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Clang HANDLE_COMPONENTS)
