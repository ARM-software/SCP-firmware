#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include_guard()

#[=======================================================================[.rst:
------------
FindYamllint

Find yamllint, the YAML linter.

Use this module by invoking ``find_package`` with the form::

  find_package(Yamllint [REQUIRED])

Imported Targets
^^^^^^^^^^^^^^^^

``Yamllint``
  ``yamllint``. Target defined if ``yamllint`` is found.

Result Variables
^^^^^^^^^^^^^^^^

``Yamllint_FOUND``
  If false, yamllint was not found.

``Yamllint_EXECUTABLE``
  The full path to ``yamllint``.
#]=======================================================================]

# cmake-lint: disable=C0103

find_program(Yamllint_EXECUTABLE yamllint)

if(Yamllint_EXECUTABLE)
    mark_as_advanced(Yamllint_EXECUTABLE)

    add_executable(Yamllint IMPORTED)

    set_target_properties(Yamllint
                          PROPERTIES IMPORTED_LOCATION "${Yamllint_EXECUTABLE}")
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Yamllint DEFAULT_MSG Yamllint_EXECUTABLE)
