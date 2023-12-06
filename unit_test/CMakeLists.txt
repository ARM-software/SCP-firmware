#
# Arm SCP/MCP Software
# Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
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

#Append common unit tests below here (alphabetical order)
list(APPEND UNIT_MODULE amu_mmap)
list(APPEND UNIT_MODULE amu_smcf_drv)
list(APPEND UNIT_MODULE atu)
list(APPEND UNIT_MODULE dvfs)
list(APPEND UNIT_MODULE fch_polled)
list(APPEND UNIT_MODULE mhu3)
list(APPEND UNIT_MODULE mpmm)
list(APPEND UNIT_MODULE optee/mbx)
list(APPEND UNIT_MODULE pl011)
list(APPEND UNIT_MODULE power_domain)
list(APPEND UNIT_MODULE ppu_v1)
list(APPEND UNIT_MODULE resource_perms)
list(APPEND UNIT_MODULE scmi)
list(APPEND UNIT_MODULE scmi_clock)
list(APPEND UNIT_MODULE scmi_perf)
list(APPEND UNIT_MODULE scmi_power_capping)
list(APPEND UNIT_MODULE scmi_sensor_req)
list(APPEND UNIT_MODULE scmi_system_power)
list(APPEND UNIT_MODULE scmi_system_power_req)
list(APPEND UNIT_MODULE sensor_smcf_drv)
list(APPEND UNIT_MODULE smcf)
list(APPEND UNIT_MODULE thermal_mgmt)
list(APPEND UNIT_MODULE traffic_cop)
list(APPEND UNIT_MODULE xr77128)

list(LENGTH UNIT_MODULE UNIT_TEST_MAX)

enable_testing()
include(CTest)

# cmake-lint: disable=E1120
foreach(idx RANGE ${UNIT_TEST_MAX})
    if(idx EQUAL UNIT_TEST_MAX)
        break()
    endif()

    list(GET UNIT_MODULE ${idx} TEST_MODULE)

    add_subdirectory(${SCP_ROOT}/module/${TEST_MODULE}/test
        ${CMAKE_BINARY_DIR}/${TEST_MODULE})

endforeach()
