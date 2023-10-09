#
# Arm SCP/MCP Software
# Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Default flags target
set(TEST_SRC mod_sensor_smcf_drv)
set(TEST_FILE mod_sensor_smcf_drv)

if(TEST_ON_TARGET)
    set(TEST_MODULE sensor_smcf_drv)
    set(MODULE_ROOT ${CMAKE_SOURCE_DIR}/module)
else()
    set(UNIT_TEST_TARGET mod_${TEST_MODULE}_unit_test)
endif()

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/smcf/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/smcf/src)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/sensor/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/sensor/src)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/smcf/test/mocks)
set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_mm)
list(APPEND MOCK_REPLACEMENTS fwk_id)

include(${SCP_ROOT}/unit_test/module_common.cmake)
