#
# Arm SCP/MCP Software
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Default flags target
set(TEST_SRC mod_amu_smcf_drv)
set(TEST_FILE mod_amu_smcf_drv)

if(TEST_ON_TARGET)
    set(TEST_MODULE amu_smcf_drv)
    set(MODULE_ROOT ${CMAKE_SOURCE_DIR}/module)
else()
    set(UNIT_TEST_TARGET mod_${TEST_MODULE}_unit_test)
endif()

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)
list(APPEND OTHER_MODULE_INC ${SCP_ROOT}/interface/amu)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/smcf/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/smcf/src)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/smcf/test/mocks)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/timer/include)
set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_mm)

include(${SCP_ROOT}/unit_test/module_common.cmake)
