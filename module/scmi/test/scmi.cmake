#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

if(TEST_ON_TARGET)
    set(TEST_MODULE scmi)
    set(MODULE_ROOT ${CMAKE_SOURCE_DIR}/module)
else()
    set(UNIT_TEST_TARGET mod_${TEST_MODULE}_unit_test)
endif()

set(MODULE_SRC ${MODULE_ROOT}/${TEST_MODULE}/src)
set(MODULE_INC ${MODULE_ROOT}/${TEST_MODULE}/include)
set(MODULE_UT_SRC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR})
set(MODULE_UT_MOCK_SRC ${CMAKE_CURRENT_LIST_DIR}/mocks)

if(TEST_ON_HOST)
    # Duplicate list of framework sources to be added to build
    foreach(fwk_src IN LISTS FWK_SRC)
        list(APPEND ${TEST_MODULE}_FWK_SRC ${fwk_src})
    endforeach()

    # Create unit test target
    add_executable(${UNIT_TEST_TARGET}
                   ${MODULE_UT_SRC}/mod_${TEST_MODULE}_unit_test.c)
endif()

if(TEST_ON_TARGET)
    # Add sources to test target
    target_sources(${UNIT_TEST_TARGET}
                  PRIVATE ${MODULE_UT_SRC}/mod_${TEST_MODULE}_unit_test.c)

    target_compile_definitions(
        ${UNIT_TEST_TARGET}
        PUBLIC "UNITY_INCLUDE_CONFIG_H=\"unity_config.h\""
        PUBLIC "TEST_ON_TARGET")
endif()

# Include test module source in unit test c source file.
target_compile_definitions(
    ${UNIT_TEST_TARGET}
    PUBLIC "UNIT_TEST_SRC=\"${MODULE_SRC}/mod_${TEST_MODULE}.c\"")


target_include_directories(
    ${UNIT_TEST_TARGET}
    PRIVATE ${MODULE_SRC}
            ${MODULE_INC}
            ${UNITY_SRC_ROOT}
            ${CMOCK_SRC_ROOT}
            ${MODULE_UT_SRC}
            ${MODULE_UT_INC}
            ${MODULE_UT_MOCK_SRC})

if(EXISTS ${MODULE_INC}/internal)
    target_include_directories(${UNIT_TEST_TARGET}
                               PRIVATE ${MODULE_INC}/internal)
endif()

if(EXISTS ${MODULE_UT_MOCK_SRC}/Mockmod_${TEST_MODULE}_extra.c)
    target_sources(${UNIT_TEST_TARGET}
                   PRIVATE ${MODULE_UT_MOCK_SRC}/Mockmod_${TEST_MODULE}_extra.c)
endif()

if(TEST_ON_HOST)
    target_include_directories(
        ${UNIT_TEST_TARGET}
        PRIVATE ${FWK_MOCKS_SRC_ROOT}
                ${FWK_MOCKS_SRC_ROOT}/..
        )

    if(EXISTS ${MODULE_UT_INC}/fwk_module_idx.h)
        target_compile_definitions(
            ${UNIT_TEST_TARGET}
            PRIVATE "FWK_TEST_MODULE_IDX_H=\"${${TEST_TARGET}_MODULE_IDX_H}\"")
    endif()

    replace_with_mock(fwk_module)
    replace_with_mock(fwk_id)

    target_sources(${UNIT_TEST_TARGET} PRIVATE ${${TEST_MODULE}_FWK_SRC})
endif()
