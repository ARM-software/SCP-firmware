#
# Arm SCP/MCP Software
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Default flags target

set(TEST_SRC mod_resource_perms)
set(TEST_FILE mod_resource_perms)

add_compile_definitions(BUILD_HAS_MOD_SCMI_RESET_DOMAIN)

if(TEST_ON_TARGET)
    set(TEST_MODULE resource_perms)
    set(MODULE_ROOT ${CMAKE_SOURCE_DIR}/module)
else()
    set(UNIT_TEST_TARGET mod_${TEST_MODULE}_unit_test)
endif()

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/scmi/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/resource_perms/include)
set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND MOCK_REPLACEMENTS fwk_mm)
list(APPEND MOCK_REPLACEMENTS fwk_string)
list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_core)

include(${SCP_ROOT}/unit_test/module_common.cmake)
