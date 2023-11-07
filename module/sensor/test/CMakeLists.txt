#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Default flags target

set(TEST_SRC mod_sensor)
set(TEST_FILE mod_sensor)

set(UNIT_TEST_TARGET mod_${TEST_MODULE}_unit_test)

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)
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

# Target with following definitions:
# BUILD_HAS_SENSOR_MULTI_AXIS
# BUILD_HAS_SENSOR_TIMESTAMP

set(TEST_SRC mod_sensor)
set(TEST_FILE mod_sensor_with_conditionals)

set(UNIT_TEST_TARGET mod_${TEST_MODULE}_with_conditionals_unit_test)

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)

list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/sensor/include)

set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND MOCK_REPLACEMENTS fwk_mm)
list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_core)
list(APPEND MOCK_REPLACEMENTS fwk_status)
list(APPEND MOCK_REPLACEMENTS fwk_string)
list(APPEND MOCK_REPLACEMENTS fwk_time)

include(${SCP_ROOT}/unit_test/module_common.cmake)

target_compile_definitions(${UNIT_TEST_TARGET}
        PUBLIC "BUILD_HAS_SENSOR_MULTI_AXIS")
target_compile_definitions(${UNIT_TEST_TARGET}
        PUBLIC "BUILD_HAS_SENSOR_TIMESTAMP")

# Target with following definitions:
# BUILD_HAS_SENSOR_MULTI_AXIS
# BUILD_HAS_SENSOR_TIMESTAMP
# Note that this build tests the "sensor_extended" source file.

set(TEST_SRC sensor_extended)
set(TEST_FILE mod_sensor_with_timestamp)

set(UNIT_TEST_TARGET mod_${TEST_MODULE}_unit_test_with_timestamp)

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)

list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/sensor/include)

set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND MOCK_REPLACEMENTS fwk_mm)
list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_core)
list(APPEND MOCK_REPLACEMENTS fwk_status)
list(APPEND MOCK_REPLACEMENTS fwk_string)
list(APPEND MOCK_REPLACEMENTS fwk_time)

include(${SCP_ROOT}/unit_test/module_common.cmake)

target_compile_definitions(${UNIT_TEST_TARGET}
        PUBLIC "BUILD_HAS_SENSOR_MULTI_AXIS")
target_compile_definitions(${UNIT_TEST_TARGET}
        PUBLIC "BUILD_HAS_SENSOR_TIMESTAMP")

# Target with following definitions:
# BUILD_HAS_SENSOR_MULTI_AXIS
# BUILD_HAS_SENSOR_TIMESTAMP
# Note that this build tests the "sensor_extended" source file.

set(TEST_SRC sensor_extended)
set(TEST_FILE mod_sensor_with_axis)

set(UNIT_TEST_TARGET mod_${TEST_MODULE}_unit_test_with_axis)

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)

list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/sensor/include)

set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND MOCK_REPLACEMENTS fwk_mm)
list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_core)
list(APPEND MOCK_REPLACEMENTS fwk_status)
list(APPEND MOCK_REPLACEMENTS fwk_string)
list(APPEND MOCK_REPLACEMENTS fwk_time)

include(${SCP_ROOT}/unit_test/module_common.cmake)

target_compile_definitions(${UNIT_TEST_TARGET}
        PUBLIC "BUILD_HAS_SENSOR_MULTI_AXIS")
target_compile_definitions(${UNIT_TEST_TARGET}
        PUBLIC "BUILD_HAS_SENSOR_TIMESTAMP")
