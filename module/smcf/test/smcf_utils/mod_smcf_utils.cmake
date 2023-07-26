#
# Arm SCP/MCP Software
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(TEST_SRC smcf_utils)
set(TEST_FILE smcf_utils)

if(TEST_ON_TARGET)
    set(TEST_MODULE smcf)
    set(MODULE_ROOT ${CMAKE_SOURCE_DIR}/module)
else()
    set(TEST_FILE smcf_utils)
    set(UNIT_TEST_TARGET ${TEST_FILE}_unit_test)
endif()

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)
set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})

list(APPEND MOCK_REPLACEMENTS fwk_module)

if(NOT TEST_ON_TARGET)
    set(TEST_FILE smcf_utils)
    set(UNIT_TEST_TARGET ${TEST_FILE}_unit_test)
endif()

Include(${SCP_ROOT}/unit_test/module_common.cmake)
