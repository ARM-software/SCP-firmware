#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(SCP_ROOT ${CMAKE_SOURCE_DIR})
set(CONTRIB_ROOT ${CMAKE_SOURCE_DIR}/contrib)
set(UNITY_SRC_ROOT ${CONTRIB_ROOT}/cmock/git/vendor/unity/src)
set(CMOCK_SRC_ROOT ${CONTRIB_ROOT}/cmock/git/src)

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(${SCP_MODULE_TARGET}
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_ut.c")

set(UNIT_TEST_TARGET ${SCP_MODULE_TARGET})
add_subdirectory(${CMAKE_SOURCE_DIR}/module/scmi/test
    ${CMAKE_BINARY_DIR}/module/scmi/test)

set(UNIT_TEST_ROOT ${CMAKE_SOURCE_DIR}/unit_test)

target_include_directories(
    ${UNIT_TEST_TARGET} PRIVATE ${UNIT_TEST_ROOT}/unity_mocks ${CMOCK_SRC_ROOT}
                                ${UNITY_SRC_ROOT})
target_compile_definitions(${UNIT_TEST_TARGET} PRIVATE -DTEST_TARGET=1)
target_sources(
    ${UNIT_TEST_TARGET}
    PRIVATE ${UNITY_SRC_ROOT}/unity.c ${CMOCK_SRC_ROOT}/cmock.c
            ${UNIT_TEST_ROOT}/unity_mocks/scp_unity.c)
