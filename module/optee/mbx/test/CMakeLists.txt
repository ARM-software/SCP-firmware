#
# Arm SCP/MCP Software
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(TEST_SRC mod_optee_mbx)
set(TEST_FILE mod_optee_mbx)

# set(UNIT_TEST_TARGET mod_${TEST_MODULE}_unit_test) This setting will not work
# as TEST_MODULE in that case will be optee/mbx which will not be parsed
# correctly. Hence, setting the UNIT_TEST_TARGET explicitly as follows
set(UNIT_TEST_TARGET mod_optee_mbx_unit_test)

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/optee/mbx/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/optee/smt/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/msg_smt/include)
set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR}/ext)
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_core)
list(APPEND MOCK_REPLACEMENTS fwk_mm)

include(${SCP_ROOT}/unit_test/module_common.cmake)

target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "BUILD_HAS_MOD_OPTEE_SMT")
target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "BUILD_HAS_MOD_MSG_SMT")
