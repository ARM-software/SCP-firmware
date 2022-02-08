#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.18.3)

project(
    SCP_UNIT_TEST
    VERSION 2.8.0
    DESCRIPTION "Arm SCP/MCP Software"
    HOMEPAGE_URL
        "https://developer.arm.com/tools-and-software/open-source-software/firmware/scp-firmware"
    LANGUAGES C ASM)

find_package(Git)

if(LOG_LEVEL)
    set(SCP_LOG_LEVEL
        ${LOG_LEVEL}
        CACHE STRING "Set SCP Firmware log Level")
endif()

set(SCP_UNIT_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SCP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../")

if(GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" describe --tags --dirty --always
        WORKING_DIRECTORY "${SCP_UNIT_TEST_DIR}"
        OUTPUT_VARIABLE SCP_UNIT_TEST_DESCRIBE
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if(NOT SCP_FWK_TEST_DESCRIBE)
    set(SCP_FWK_TEST_DESCRIBE "v${SCP_FWK_TEST_VERSION}-<unknown>")
endif()

# Setup top level paths.

# Curent test directory
set(TEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Common framework sources
set(FWK_SRC_ROOT ${SCP_ROOT}/framework/src)
set(FWK_INC_ROOT ${SCP_ROOT}/framework/include)

# CMSIS
set(CMSIS_ROOT ${SCP_ROOT}/contrib/cmsis/git/CMSIS/RTOS2)

# Source module directory
set(MODULE_ROOT ${SCP_ROOT}/module)

# Unity and CMock sources
set(CMOCK_ROOT ${SCP_ROOT}/contrib/cmock/git)
set(UNITY_ROOT ${CMOCK_ROOT}/vendor/unity)
set(UNITY_SRC_ROOT ${UNITY_ROOT}/src)
set(CMOCK_SRC_ROOT ${CMOCK_ROOT}/src)

# Framework mock sources
set(FWK_MOCKS_SRC_ROOT ${TEST_ROOT}/unity_mocks/mocks)

set(TEST_ON_HOST TRUE)

# Common flags
list(APPEND EXTRA_COMPILE_FLAGS -g3)
list(APPEND EXTRA_COMPILE_FLAGS -Wall)
list(APPEND EXTRA_COMPILE_FLAGS -Wextra)
list(APPEND EXTRA_COMPILE_FLAGS -Werror)
list(APPEND EXTRA_COMPILE_FLAGS -Wno-missing-field-initializers)
list(APPEND EXTRA_COMPILE_FLAGS -Wno-error=deprecated-declarations)
list(APPEND EXTRA_COMPILE_FLAGS -Wno-unused-parameter)
list(APPEND EXTRA_COMPILE_FLAGS -Wno-strict-aliasing)
list(APPEND EXTRA_COMPILE_FLAGS -std=gnu11)
list(APPEND EXTRA_COMPILE_FLAGS -DBUILD_MODE_DEBUG)
list(APPEND EXTRA_COMPILE_FLAGS -DBUILD_TESTS)
list(APPEND EXTRA_COMPILE_FLAGS -p)
list(APPEND EXTRA_COMPILE_FLAGS --coverage)
if(SCP_LOG_LEVEL)
    list(APPEND EXTRA_COMPILE_FLAGS -DFWK_LOG_LEVEL=${SCP_LOG_LEVEL})
endif()

set(CMAKE_CXX_FLAGS_DEBUG_INIT ${EXTRA_COMPILE_CFLAGS_STR})

# Create a list of common framework sources.
list(APPEND FWK_SRC ${CMOCK_SRC_ROOT}/cmock.c)
list(APPEND FWK_SRC ${FWK_SRC_ROOT}/fwk_arch.c)
list(APPEND FWK_SRC ${FWK_SRC_ROOT}/fwk_core.c)
list(APPEND FWK_SRC ${FWK_SRC_ROOT}/fwk_dlist.c)
list(APPEND FWK_SRC ${FWK_SRC_ROOT}/fwk_id.c)
list(APPEND FWK_SRC ${FWK_SRC_ROOT}/fwk_interrupt.c)
list(APPEND FWK_SRC ${FWK_SRC_ROOT}/fwk_io.c)
list(APPEND FWK_SRC ${FWK_SRC_ROOT}/fwk_log.c)
list(APPEND FWK_SRC ${FWK_SRC_ROOT}/fwk_mm.c)
list(APPEND FWK_SRC ${FWK_SRC_ROOT}/fwk_module.c)
list(APPEND FWK_SRC ${FWK_SRC_ROOT}/fwk_ring.c)
list(APPEND FWK_SRC ${FWK_SRC_ROOT}/fwk_slist.c)
list(APPEND FWK_SRC ${FWK_SRC_ROOT}/fwk_status.c)
list(APPEND FWK_SRC ${FWK_SRC_ROOT}/fwk_string.c)
list(APPEND FWK_SRC ${FWK_SRC_ROOT}/fwk_delayed_resp.c)
list(APPEND FWK_SRC ${FWK_SRC_ROOT}/fwk_time.c)
list(APPEND FWK_SRC ${UNITY_SRC_ROOT}/unity.c)

list(APPEND SCP_UNITY_SRC ${TEST_ROOT}/unity_mocks/scp_unity.c)

macro(replace_with_mock FILENAME)
    list(FILTER ${TEST_MODULE}_FWK_SRC EXCLUDE REGEX "${FILENAME}.c")
    list(APPEND ${TEST_MODULE}_FWK_SRC ${FWK_MOCKS_SRC_ROOT}/Mock${FILENAME}.c)
    if(EXISTS ${FWK_MOCKS_SRC_ROOT}/internal/Mock${FILENAME}_internal.c)
        list(APPEND ${TEST_MODULE}_FWK_SRC
                    ${FWK_MOCKS_SRC_ROOT}/internal/Mock${FILENAME}_internal.c)
    endif()
endmacro()

macro(include_fwk_notify)
    target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "BUILD_HAS_NOTIFICATION")
    list(APPEND ${TEST_MODULE}_FWK_SRC ${FWK_SRC_ROOT}/fwk_notification.c)
endmacro()

macro(include_mock_notify)
    target_compile_definitions(${UNIT_TEST_TARGET} PUBLIC "BUILD_HAS_NOTIFICATION")
    list(APPEND ${TEST_MODULE}_FWK_SRC ${FWK_MOCKS_SRC_ROOT}/Mockfwk_notification.c)
    list(APPEND ${TEST_MODULE}_FWK_SRC
                ${FWK_MOCKS_SRC_ROOT}/internal/Mockfwk_notification_internal.c)
endmacro()

#Append new unit tests below here
list(APPEND UNIT_MODULE scmi)

list(LENGTH UNIT_MODULE UNIT_TEST_MAX)

enable_testing()
include(CTest)

# cmake-lint: disable=E1120
foreach(idx RANGE ${UNIT_TEST_MAX})
    if(idx EQUAL UNIT_TEST_MAX)
        break()
    endif()

    list(GET UNIT_MODULE ${idx} TEST_MODULE)

    # Include .cmake file of the module under test
    include(${SCP_ROOT}/module/${TEST_MODULE}/test/${TEST_MODULE}.cmake)

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

endforeach()
