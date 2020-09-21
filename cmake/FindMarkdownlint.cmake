#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include_guard()

#[=======================================================================[.rst:
------------
FindMarkdownlint

Find markdownlint, the Markdown linter.

Use this module by invoking ``find_package`` with the form::

  find_package(Markdownlint [REQUIRED])

Imported Targets
^^^^^^^^^^^^^^^^

``Markdownlint``
  ``mdl``. Target defined if markdownlint is found.

Result Variables
^^^^^^^^^^^^^^^^

``Markdownlint_FOUND``
  If false, ``mdl`` was not found.

``Markdownlint_EXECUTABLE``
  The full path to ``mdl``.
#]=======================================================================]

# cmake-lint: disable=C0103

find_program(Markdownlint_EXECUTABLE "mdl")

if(Markdownlint_EXECUTABLE)
    mark_as_advanced(Markdownlint_EXECUTABLE)

    add_executable(Markdownlint IMPORTED)

    set_target_properties(
        Markdownlint PROPERTIES IMPORTED_LOCATION "${Markdownlint_EXECUTABLE}")
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Markdownlint DEFAULT_MSG
                                  Markdownlint_EXECUTABLE)
