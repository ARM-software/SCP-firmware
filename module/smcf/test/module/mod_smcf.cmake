#
# Arm SCP/MCP Software
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(TEST_SRC mod_smcf)
set(TEST_FILE interface)

if(TEST_ON_TARGET)
    set(TEST_MODULE smcf)
    set(MODULE_ROOT ${CMAKE_SOURCE_DIR}/module)
else()
    set(TEST_FILE interface)
    set(UNIT_TEST_TARGET ${TEST_FILE}_unit_test)
endif()

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)
set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/../mocks)

list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_mm)
list(APPEND MOCK_REPLACEMENTS fwk_interrupt)

include(${SCP_ROOT}/unit_test/module_common.cmake)

target_sources(${UNIT_TEST_TARGET}
    PRIVATE
        ${MODULE_UT_MOCK_SRC}/Mockmgi.c
        ${MODULE_UT_MOCK_SRC}/Mocksmcf_data.c)
