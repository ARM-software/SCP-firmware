#
# Arm SCP/MCP Software
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

if(TEST_ON_TARGET)
    set(TEST_MODULE scmi_power_capping)
    set(MODULE_ROOT ${CMAKE_SOURCE_DIR}/module)
endif()

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/scmi/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/power_allocator/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/power_meter/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/power_coordinator/include)
set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

################################################################################
# Test mod_scmi_power_capping.c                                                #
################################################################################
set(TEST_SRC mod_scmi_power_capping)
set(TEST_FILE mod_scmi_power_capping)

list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_core)
list(APPEND MOCK_REPLACEMENTS fwk_mm)

if(NOT TEST_ON_TARGET)
    set(UNIT_TEST_TARGET mod_${TEST_MODULE}_unit_test)
endif()

include(${SCP_ROOT}/unit_test/module_common.cmake)
target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "BUILD_HAS_SCMI_POWER_CAPPING_STD_COMMANDS")

target_sources(${UNIT_TEST_TARGET}
    PRIVATE
        ${MODULE_UT_MOCK_SRC}/Mockscmi_power_capping_protocol.c)

################################################################################
# Test scmi_power_capping_protocol.c                                           #
################################################################################
set(TEST_SRC scmi_power_capping_protocol)
set(TEST_FILE scmi_power_capping_protocol)

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/scmi/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/resource_perms/include)
set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_core)
list(APPEND MOCK_REPLACEMENTS fwk_mm)
list(APPEND MOCK_REPLACEMENTS fwk_notification)

if(NOT TEST_ON_TARGET)
    set(UNIT_TEST_TARGET mod_${TEST_MODULE}_protocol_unit_test)
endif()

include(${SCP_ROOT}/unit_test/module_common.cmake)
target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "BUILD_HAS_SCMI_POWER_CAPPING_STD_COMMANDS")

target_sources(${UNIT_TEST_TARGET}
    PRIVATE
        ${MODULE_UT_MOCK_SRC}/Mockmod_scmi_extra.c
        ${MODULE_UT_MOCK_SRC}/Mockmod_power_allocator_extra.c
        ${MODULE_UT_MOCK_SRC}/Mockmod_power_coordinator_extra.c
        ${MODULE_UT_MOCK_SRC}/Mockmod_power_meter_extra.c)

################################################################################
# Test scmi_power_capping_protocol.c + resource permissions                    #
################################################################################
set(TEST_SRC scmi_power_capping_protocol)
set(TEST_FILE scmi_power_capping_protocol)

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/scmi/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/resource_perms/include)
set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_core)
list(APPEND MOCK_REPLACEMENTS fwk_mm)
list(APPEND MOCK_REPLACEMENTS fwk_notification)

if(NOT TEST_ON_TARGET)
    set(UNIT_TEST_TARGET mod_${TEST_MODULE}_protocol_resource_permissions_unit_test)
endif()

include(${SCP_ROOT}/unit_test/module_common.cmake)
target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "BUILD_HAS_SCMI_POWER_CAPPING_STD_COMMANDS")
target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "BUILD_HAS_MOD_RESOURCE_PERMS")

target_sources(${UNIT_TEST_TARGET}
    PRIVATE
        ${MODULE_UT_MOCK_SRC}/Mockmod_scmi_extra.c
        ${MODULE_UT_MOCK_SRC}/Mockmod_power_allocator_extra.c
        ${MODULE_UT_MOCK_SRC}/Mockmod_power_coordinator_extra.c
        ${MODULE_UT_MOCK_SRC}/Mockmod_power_meter_extra.c
        ${MODULE_UT_MOCK_SRC}/Mockmod_resource_perms_extra.c)

################################################################################
# Test scmi_power_capping_protocol.c + notifications                           #
################################################################################
set(TEST_SRC scmi_power_capping_protocol)
set(TEST_FILE scmi_power_capping_protocol)

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/scmi/include)
set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_core)
list(APPEND MOCK_REPLACEMENTS fwk_mm)
list(APPEND MOCK_REPLACEMENTS fwk_notification)

if(NOT TEST_ON_TARGET)
    set(UNIT_TEST_TARGET mod_${TEST_MODULE}_protocol_notification_unit_test)
endif()

include(${SCP_ROOT}/unit_test/module_common.cmake)
target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "BUILD_HAS_SCMI_POWER_CAPPING_STD_COMMANDS")
target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "BUILD_HAS_SCMI_NOTIFICATIONS")

target_sources(${UNIT_TEST_TARGET}
    PRIVATE
        ${MODULE_UT_MOCK_SRC}/Mockmod_scmi_extra.c
        ${MODULE_UT_MOCK_SRC}/Mockmod_power_allocator_extra.c
        ${MODULE_UT_MOCK_SRC}/Mockmod_power_coordinator_extra.c
        ${MODULE_UT_MOCK_SRC}/Mockmod_power_meter_extra.c)

################################################################################
# Test scmi_power_capping_fast_channels.c with standard protocol commands      #
################################################################################
set(TEST_SRC scmi_power_capping_fast_channels)
set(TEST_FILE scmi_power_capping_fast_channels)

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/scmi/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/transport/include)
set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_core)
list(APPEND MOCK_REPLACEMENTS fwk_mm)

if(NOT TEST_ON_TARGET)
    set(UNIT_TEST_TARGET ${TEST_MODULE}_fast_channels_unit_test)
endif()

include(${SCP_ROOT}/unit_test/module_common.cmake)
target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "BUILD_HAS_SCMI_POWER_CAPPING_STD_COMMANDS")
target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "BUILD_HAS_FAST_CHANNELS")
target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "BUILD_HAS_SCMI_POWER_CAPPING_FAST_CHANNELS_COMMANDS")

target_sources(${UNIT_TEST_TARGET}
    PRIVATE
        ${MODULE_UT_MOCK_SRC}/Mockmod_power_allocator_extra.c
        ${MODULE_UT_MOCK_SRC}/Mockmod_power_coordinator_extra.c
        ${MODULE_UT_MOCK_SRC}/Mockmod_transport_extra.c)

################################################################################
# Test scmi_power_capping_fast_channels.c without standard protocol commands   #
################################################################################
set(TEST_SRC scmi_power_capping_fast_channels)
set(TEST_FILE scmi_power_capping_fast_channels)

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/scmi/include)
list(APPEND OTHER_MODULE_INC ${MODULE_ROOT}/transport/include)
set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

list(APPEND MOCK_REPLACEMENTS fwk_module)
list(APPEND MOCK_REPLACEMENTS fwk_id)
list(APPEND MOCK_REPLACEMENTS fwk_core)
list(APPEND MOCK_REPLACEMENTS fwk_mm)

if(NOT TEST_ON_TARGET)
    set(UNIT_TEST_TARGET ${TEST_MODULE}_fast_channels_only_unit_test)
endif()

include(${SCP_ROOT}/unit_test/module_common.cmake)
target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "BUILD_HAS_FAST_CHANNELS")
target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "BUILD_HAS_SCMI_POWER_CAPPING_FAST_CHANNELS_COMMANDS")

target_sources(${UNIT_TEST_TARGET}
    PRIVATE
        ${MODULE_UT_MOCK_SRC}/Mockmod_power_allocator_extra.c
        ${MODULE_UT_MOCK_SRC}/Mockmod_power_coordinator_extra.c
        ${MODULE_UT_MOCK_SRC}/Mockmod_transport_extra.c)
