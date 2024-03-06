#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# cmake-lint: disable=C0301

#
# Add the common optee modules in the SCP_MODULE_PATHS.
#
# The module paths are listed in an alphabetical order.
#

list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/module/clock")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/module/console")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/module/mbx")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/module/reset")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/module/smt")
list(PREPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/module/voltd_regulator")

set(SCP_MODULE_PATHS ${SCP_MODULE_PATHS} PARENT_SCOPE)
