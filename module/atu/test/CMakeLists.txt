#
# Arm SCP/MCP Software
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(TEST_SRC mod_atu)
set(TEST_FILE mod_atu)

set(UNIT_TEST_TARGET mod_${TEST_MODULE}_unit_test)

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)

set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})

list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_interrupt)

include(${SCP_ROOT}/unit_test/module_common.cmake)

target_compile_definitions(${UNIT_TEST_TARGET} PRIVATE BUILD_HAS_ATU_MANAGE)
