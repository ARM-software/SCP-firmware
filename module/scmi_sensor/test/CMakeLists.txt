#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Default Flags Target

set(TEST_SRC mod_scmi_sensor)
set(TEST_FILE mod_scmi_sensor)

set(UNIT_TEST_TARGET mod_${TEST_MODULE}_unit_test)

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)

list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/scmi/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/sensor/include)

set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND MOCK_REPLACEMENTS fwk_core)
list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_mm)
list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_status)
list(APPEND MOCK_REPLACEMENTS fwk_string)
list(APPEND MOCK_REPLACEMENTS fwk_time)

include(${SCP_ROOT}/unit_test/module_common.cmake)

# Target with following definition:
# with BUILD_HAS_SCMI_NOTIFICATIONS

set(TEST_SRC mod_scmi_sensor)
set(TEST_FILE mod_scmi_sensor_with_notifications)

set(UNIT_TEST_TARGET mod_${TEST_MODULE}_unit_test_with_notifications)

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)

list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/scmi/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/sensor/include)

set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND MOCK_REPLACEMENTS fwk_core)
list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_mm)
list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_status)
list(APPEND MOCK_REPLACEMENTS fwk_string)
list(APPEND MOCK_REPLACEMENTS fwk_time)

include(${SCP_ROOT}/unit_test/module_common.cmake)

target_compile_definitions(${UNIT_TEST_TARGET}
        PUBLIC "BUILD_HAS_SCMI_NOTIFICATIONS")

# Target with following definitions:
# with BUILD_HAS_MOD_RESOURCE_PERMS

set(TEST_SRC mod_scmi_sensor)
set(TEST_FILE mod_scmi_sensor_with_resource_perms)

set(UNIT_TEST_TARGET mod_${TEST_MODULE}_unit_test_with_resource_perms)

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)

list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/scmi/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/sensor/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/resource_perms/include)

set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND MOCK_REPLACEMENTS fwk_core)
list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_mm)
list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_status)
list(APPEND MOCK_REPLACEMENTS fwk_string)
list(APPEND MOCK_REPLACEMENTS fwk_time)

include(${SCP_ROOT}/unit_test/module_common.cmake)

target_compile_definitions(${UNIT_TEST_TARGET}
        PUBLIC "BUILD_HAS_MOD_RESOURCE_PERMS")
