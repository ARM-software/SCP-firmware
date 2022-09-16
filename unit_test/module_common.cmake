#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

if(TEST_ON_HOST)
    # Duplicate list of framework sources to be added to build
    foreach(fwk_src IN LISTS FWK_SRC)
        list(APPEND ${TEST_FILE}_FWK_SRC ${fwk_src})
    endforeach()

    # Duplicate list of framework mock sources to be added to build
    foreach(fwk_mock_src IN LISTS FWK_MOCK_SRC)
        list(APPEND ${TEST_FILE}_FWK_MOCK_SRC ${fwk_mock_src})
    endforeach()

    # Create unit test target
    add_executable(${UNIT_TEST_TARGET}
                   ${MODULE_UT_SRC}/${TEST_FILE}_unit_test.c)
endif()

if(TEST_ON_TARGET)
    # Add sources to test target
    target_sources(${UNIT_TEST_TARGET}
                  PRIVATE ${MODULE_UT_SRC}/${TEST_FILE}_unit_test.c)

    target_compile_definitions(
        ${UNIT_TEST_TARGET}
        PUBLIC "UNITY_INCLUDE_CONFIG_H=\"unity_config.h\""
        PUBLIC "TEST_ON_TARGET")
endif()

# Include test module source in unit test c source file.
target_compile_definitions(
    ${UNIT_TEST_TARGET}
    PUBLIC "UNIT_TEST_SRC=\"${MODULE_SRC}/${TEST_SRC}.c\"")


target_include_directories(
    ${UNIT_TEST_TARGET}
    PRIVATE ${MODULE_SRC}
            ${MODULE_INC}
            ${OTHER_MODULE_INC}
            ${UNITY_SRC_ROOT}
            ${CMOCK_SRC_ROOT}
            ${MODULE_UT_SRC}
            ${MODULE_UT_INC}
            ${MODULE_UT_MOCK_SRC})

if(EXISTS ${MODULE_INC}/internal)
    target_include_directories(${UNIT_TEST_TARGET}
                               PRIVATE ${MODULE_INC}/internal)
endif()

if(EXISTS ${MODULE_UT_MOCK_SRC}/Mock${TEST_FILE}_extra.c)
    target_sources(${UNIT_TEST_TARGET}
                   PRIVATE ${MODULE_UT_MOCK_SRC}/Mock${TEST_FILE}_extra.c)
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
            PRIVATE "FWK_TEST_FILE_IDX_H=\"${${TEST_TARGET}_MODULE_IDX_H}\"")
    endif()

    macro(replace_with_mock FILENAME)
        list(FILTER ${TEST_FILE}_FWK_SRC EXCLUDE REGEX "${FILENAME}.c")
        list(APPEND ${TEST_FILE}_FWK_SRC ${FWK_MOCKS_SRC_ROOT}/Mock${FILENAME}.c)
        if(EXISTS ${FWK_MOCKS_SRC_ROOT}/internal/Mock${FILENAME}_internal.c)
            list(APPEND ${TEST_FILE}_FWK_SRC
                        ${FWK_MOCKS_SRC_ROOT}/internal/Mock${FILENAME}_internal.c)
        endif()
    endmacro()

    foreach(MOCK IN LISTS MOCK_REPLACEMENTS)
        if(${MOCK} STREQUAL fwk_notify)
            target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "BUILD_HAS_NOTIFICATION")
            list(APPEND ${TEST_FILE}_FWK_SRC ${FWK_MOCKS_SRC_ROOT}/Mockfwk_notification.c)
            list(APPEND ${TEST_FILE}_FWK_SRC
                        ${FWK_MOCKS_SRC_ROOT}/internal/Mockfwk_notification_internal.c)
        elseif(${MOCK} STREQUAL fwk_core)
            replace_with_mock(fwk_core)
            list(FILTER ${TEST_FILE}_FWK_SRC EXCLUDE REGEX "fwk_delayed_resp.c")
        else()
            replace_with_mock(${MOCK})
        endif()
    endforeach()

    target_sources(${UNIT_TEST_TARGET} PRIVATE ${${TEST_FILE}_FWK_SRC}
                   ${${TEST_FILE}_FWK_MOCK_SRC})

    target_compile_definitions(
        ${UNIT_TEST_TARGET}
        PUBLIC "BUILD_VERSION_DESCRIBE_STRING=\"${SCP_UNIT_TEST_DESCRIBE}\""
               "BUILD_VERSION_MAJOR=${SCP_UNIT_TEST_VERSION_MAJOR}"
               "BUILD_VERSION_MINOR=${SCP_UNIT_TEST_VERSION_MINOR}"
               "BUILD_VERSION_PATCH=0")

    target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "PERF_OPT")

    foreach(COMPILE_FLAG IN LISTS EXTRA_COMPILE_FLAGS)
        target_compile_options(${UNIT_TEST_TARGET} PRIVATE "${COMPILE_FLAG}")
    endforeach()

    # Include framework includes
    target_include_directories(${UNIT_TEST_TARGET} PUBLIC "${FWK_INC_ROOT}")
    target_sources(${UNIT_TEST_TARGET} PUBLIC ${SCP_UNITY_SRC})
    target_link_options(${UNIT_TEST_TARGET}
                        PRIVATE "LINKER:-wrap=fwk_log_printf --coverage -lgcov")
    target_link_libraries(${UNIT_TEST_TARGET} PRIVATE gcov)
    # Add test to CTest
    add_test(NAME ${UNIT_TEST_TARGET} COMMAND ${UNIT_TEST_TARGET})
endif()

unset(MOCK_REPLACEMENTS)
