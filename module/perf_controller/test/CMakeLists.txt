#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

################################################################################
# Test mod_perf_controller main logic                                          #
################################################################################
set(TEST_SRC mod_perf_controller)
set(TEST_FILE mod_perf_controller)

if(TEST_ON_TARGET)
    set(TEST_MODULE perf_controller)
    set(MODULE_ROOT ${CMAKE_SOURCE_DIR}/module)
else()
    set(UNIT_TEST_TARGET mod_${TEST_MODULE}_unit_test)
endif()

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)
set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND OTHER_MODULE_INC ${SCP_ROOT}/interface/power_management)

include(${SCP_ROOT}/unit_test/module_common.cmake)

target_sources(${UNIT_TEST_TARGET}
    PRIVATE ${MODULE_UT_MOCK_SRC}/fwk_variables_mocks.c
    PRIVATE ${MODULE_UT_MOCK_SRC}/Mockmod_perf_controller_extra.c)

################################################################################
# Test mod_perf_controller framework handlers                                  #
################################################################################
set(TEST_SRC mod_perf_controller)
set(TEST_FILE mod_perf_controller_fwk)

if(TEST_ON_TARGET)
    set(TEST_MODULE perf_controller)
    set(MODULE_ROOT ${CMAKE_SOURCE_DIR}/module)
else()
    set(UNIT_TEST_TARGET mod_${TEST_MODULE}_fwk_unit_test)
endif()

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)
set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND MOCK_REPLACEMENTS fwk_mm)
list(APPEND MOCK_REPLACEMENTS fwk_module)

list(APPEND OTHER_MODULE_INC ${SCP_ROOT}/interface/power_management)

include(${SCP_ROOT}/unit_test/module_common.cmake)

target_sources(${UNIT_TEST_TARGET}
    PRIVATE ${MODULE_UT_MOCK_SRC}/fwk_variables_mocks.c
    PRIVATE ${MODULE_UT_MOCK_SRC}/Mockmod_perf_controller_extra.c)
