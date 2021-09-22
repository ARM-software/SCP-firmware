#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.18.3)

project(
    SCP_FWK_TEST
    VERSION 2.9.0
    DESCRIPTION "Arm SCP/MCP Software Framework tests"
    HOMEPAGE_URL
        "https://developer.arm.com/tools-and-software/open-source-software/firmware/scp-firmware"
    LANGUAGES C ASM)

set(SCP_FWK_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Get details about current source version
find_package(Git)
if(GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" describe --tags --dirty --always
        WORKING_DIRECTORY "${SCP_FWK_TEST_DIR}"
        OUTPUT_VARIABLE SCP_FWK_TEST_DESCRIBE
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if(NOT SCP_FWK_TEST_DESCRIBE)
    set(SCP_FWK_TEST_DESCRIBE "v${SCP_FWK_TEST_VERSION}-<unknown>")
endif()

# Set up toplevel paths
set(FWK_TEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(FWK_SRC_ROOT ${FWK_TEST_ROOT}/../src)
set(FWK_INC_ROOT ${FWK_TEST_ROOT}/../include)
set(FWK_TEST_SRC_ROOT ${FWK_TEST_ROOT})
set(FWK_TEST_INC_ROOT ${FWK_TEST_ROOT}/include)
set(FWK_SCP_ROOT ${FWK_TEST_ROOT}/../..)
set(CMSIS_ROOT ${FWK_SCP_ROOT}/contrib/cmsis/git/CMSIS/RTOS2)

# Setup additional compiler flags
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

# Add test targets
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_id_equality)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_id_get_idx)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_id_type)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_interrupt)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_list_contains)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_list_empty)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_list_get)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_list_head)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_list_init)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_list_insert)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_list_next)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_list_pop)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_list_push)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_list_remove)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_macros)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_math)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_module)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_multi_thread_common_thread)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_multi_thread_create)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_multi_thread_init)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_multi_thread_put_event)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_multi_thread_util)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_notification)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_ring)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_ring_init)
list(APPEND SCP_FWK_TEST_TARGETS test_fwk_thread)

# Create a list of the tests that need notifications.
list(APPEND NOTIFICATION_ENABLED_TEST test_fwk_module test_fwk_notification
     test_fwk_thread)

# Create a list of the tests that need multi-threading
list(
    APPEND
    MULTITHREADING_ENABLED_TESTS
    test_fwk_multi_thread_common_thread
    test_fwk_multi_thread_create
    test_fwk_multi_thread_init
    test_fwk_multi_thread_put_event
    test_fwk_multi_thread_util)

# Some test may need its own implementation of some of the function
# for testing purpose. Create a list per test of these functions.
list(APPEND test_fwk_module_WRAP __fwk_notification_init)
list(APPEND test_fwk_module_WRAP __fwk_thread_init)
list(APPEND test_fwk_module_WRAP __fwk_thread_run)
list(APPEND test_fwk_module_WRAP fwk_mm_calloc)

list(APPEND test_fwk_thread_WRAP fwk_module_get_ctx)
list(APPEND test_fwk_thread_WRAP fwk_module_get_element_ctx)
list(APPEND test_fwk_thread_WRAP __fwk_slist_push_tail)
list(APPEND test_fwk_thread_WRAP fwk_mm_calloc)
list(APPEND test_fwk_thread_WRAP fwk_interrupt_get_current)
list(APPEND test_fwk_thread_WRAP fwk_interrupt_global_disable)
list(APPEND test_fwk_thread_WRAP fwk_interrupt_global_enable)
list(APPEND test_fwk_thread_WRAP fwk_module_is_valid_entity_id)
list(APPEND test_fwk_thread_WRAP fwk_module_is_valid_event_id)
list(APPEND test_fwk_thread_WRAP fwk_module_is_valid_notification_id)

list(APPEND test_fwk_notification_WRAP fwk_module_get_ctx)
list(APPEND test_fwk_notification_WRAP fwk_module_get_element_ctx)
list(APPEND test_fwk_notification_WRAP __fwk_thread_get_current_event)
list(APPEND test_fwk_notification_WRAP __fwk_thread_put_notification)
list(APPEND test_fwk_notification_WRAP fwk_mm_calloc)
list(APPEND test_fwk_notification_WRAP fwk_interrupt_get_current)
list(APPEND test_fwk_notification_WRAP fwk_interrupt_global_disable)
list(APPEND test_fwk_notification_WRAP fwk_interrupt_global_enable)
list(APPEND test_fwk_notification_WRAP fwk_module_is_valid_entity_id)
list(APPEND test_fwk_notification_WRAP fwk_module_is_valid_notification_id)

list(APPEND test_fwk_multi_thread_init_WRAP fwk_module_get_ctx)
list(APPEND test_fwk_multi_thread_init_WRAP fwk_module_get_element_ctx)
list(APPEND test_fwk_multi_thread_init_WRAP fwk_module_get_element_ctx)
list(APPEND test_fwk_multi_thread_init_WRAP fwk_module_get_state)
list(APPEND test_fwk_multi_thread_init_WRAP fwk_mm_calloc)
list(APPEND test_fwk_multi_thread_init_WRAP fwk_interrupt_get_current)
list(APPEND test_fwk_multi_thread_init_WRAP fwk_interrupt_global_disable)
list(APPEND test_fwk_multi_thread_init_WRAP fwk_interrupt_global_enable)
list(APPEND test_fwk_multi_thread_init_WRAP fwk_module_is_valid_element_id)
list(APPEND test_fwk_multi_thread_init_WRAP fwk_module_is_valid_entity_id)
list(APPEND test_fwk_multi_thread_init_WRAP fwk_module_is_valid_event_id)
list(APPEND test_fwk_multi_thread_init_WRAP fwk_module_is_valid_module_id)
list(APPEND test_fwk_multi_thread_init_WRAP fwk_module_is_valid_notification_id)
list(APPEND test_fwk_multi_thread_init_WRAP osKernelInitialize)
list(APPEND test_fwk_multi_thread_init_WRAP osKernelStart)
list(APPEND test_fwk_multi_thread_init_WRAP osThreadFlagsClear)
list(APPEND test_fwk_multi_thread_init_WRAP osThreadFlagsSet)
list(APPEND test_fwk_multi_thread_init_WRAP osThreadFlagsWait)
list(APPEND test_fwk_multi_thread_init_WRAP osThreadNew)

list(APPEND test_fwk_multi_thread_create_WRAP fwk_module_get_ctx)
list(APPEND test_fwk_multi_thread_create_WRAP fwk_module_get_element_ctx)
list(APPEND test_fwk_multi_thread_create_WRAP fwk_module_get_element_ctx)
list(APPEND test_fwk_multi_thread_create_WRAP fwk_module_get_state)
list(APPEND test_fwk_multi_thread_create_WRAP fwk_mm_calloc)
list(APPEND test_fwk_multi_thread_create_WRAP fwk_interrupt_get_current)
list(APPEND test_fwk_multi_thread_create_WRAP fwk_interrupt_global_disable)
list(APPEND test_fwk_multi_thread_create_WRAP fwk_interrupt_global_enable)
list(APPEND test_fwk_multi_thread_create_WRAP fwk_module_is_valid_element_id)
list(APPEND test_fwk_multi_thread_create_WRAP fwk_module_is_valid_entity_id)
list(APPEND test_fwk_multi_thread_create_WRAP fwk_module_is_valid_event_id)
list(APPEND test_fwk_multi_thread_create_WRAP fwk_module_is_valid_module_id)
list(APPEND test_fwk_multi_thread_create_WRAP
     fwk_module_is_valid_notification_id)
list(APPEND test_fwk_multi_thread_create_WRAP osKernelInitialize)
list(APPEND test_fwk_multi_thread_create_WRAP osKernelStart)
list(APPEND test_fwk_multi_thread_create_WRAP osThreadFlagsClear)
list(APPEND test_fwk_multi_thread_create_WRAP osThreadFlagsSet)
list(APPEND test_fwk_multi_thread_create_WRAP osThreadFlagsWait)
list(APPEND test_fwk_multi_thread_create_WRAP osThreadNew)

list(APPEND test_fwk_multi_thread_common_thread_WRAP fwk_module_get_ctx)
list(APPEND test_fwk_multi_thread_common_thread_WRAP fwk_module_get_element_ctx)
list(APPEND test_fwk_multi_thread_common_thread_WRAP fwk_module_get_state)
list(APPEND test_fwk_multi_thread_common_thread_WRAP fwk_mm_calloc)
list(APPEND test_fwk_multi_thread_common_thread_WRAP fwk_interrupt_get_current)
list(APPEND test_fwk_multi_thread_common_thread_WRAP
     fwk_interrupt_global_disable)
list(APPEND test_fwk_multi_thread_common_thread_WRAP
     fwk_interrupt_global_enable)
list(APPEND test_fwk_multi_thread_common_thread_WRAP
     fwk_module_is_valid_element_id)
list(APPEND test_fwk_multi_thread_common_thread_WRAP
     fwk_module_is_valid_entity_id)
list(APPEND test_fwk_multi_thread_common_thread_WRAP
     fwk_module_is_valid_event_id)
list(APPEND test_fwk_multi_thread_common_thread_WRAP
     fwk_module_is_valid_module_id)
list(APPEND test_fwk_multi_thread_common_thread_WRAP
     fwk_module_is_valid_notification_id)
list(APPEND test_fwk_multi_thread_common_thread_WRAP osKernelInitialize)
list(APPEND test_fwk_multi_thread_common_thread_WRAP osKernelStart)
list(APPEND test_fwk_multi_thread_common_thread_WRAP osThreadFlagsClear)
list(APPEND test_fwk_multi_thread_common_thread_WRAP osThreadFlagsSet)
list(APPEND test_fwk_multi_thread_common_thread_WRAP osThreadFlagsWait)
list(APPEND test_fwk_multi_thread_common_thread_WRAP osThreadNew)

list(APPEND test_fwk_multi_thread_put_event_WRAP fwk_module_get_ctx)
list(APPEND test_fwk_multi_thread_put_event_WRAP fwk_module_get_element_ctx)
list(APPEND test_fwk_multi_thread_put_event_WRAP fwk_module_get_element_ctx)
list(APPEND test_fwk_multi_thread_put_event_WRAP fwk_module_get_state)
list(APPEND test_fwk_multi_thread_put_event_WRAP fwk_mm_calloc)
list(APPEND test_fwk_multi_thread_put_event_WRAP fwk_interrupt_get_current)
list(APPEND test_fwk_multi_thread_put_event_WRAP fwk_interrupt_global_disable)
list(APPEND test_fwk_multi_thread_put_event_WRAP fwk_interrupt_global_enable)
list(APPEND test_fwk_multi_thread_put_event_WRAP fwk_module_is_valid_element_id)
list(APPEND test_fwk_multi_thread_put_event_WRAP fwk_module_is_valid_entity_id)
list(APPEND test_fwk_multi_thread_put_event_WRAP fwk_module_is_valid_event_id)
list(APPEND test_fwk_multi_thread_put_event_WRAP fwk_module_is_valid_module_id)
list(APPEND test_fwk_multi_thread_put_event_WRAP
     fwk_module_is_valid_notification_id)
list(APPEND test_fwk_multi_thread_put_event_WRAP osKernelInitialize)
list(APPEND test_fwk_multi_thread_put_event_WRAP osKernelStart)
list(APPEND test_fwk_multi_thread_put_event_WRAP osThreadFlagsClear)
list(APPEND test_fwk_multi_thread_put_event_WRAP osThreadFlagsSet)
list(APPEND test_fwk_multi_thread_put_event_WRAP osThreadFlagsWait)
list(APPEND test_fwk_multi_thread_put_event_WRAP osThreadNew)

list(APPEND test_fwk_multi_thread_util_WRAP fwk_module_get_ctx)
list(APPEND test_fwk_multi_thread_util_WRAP fwk_module_get_element_ctx)
list(APPEND test_fwk_multi_thread_util_WRAP fwk_module_get_element_ctx)
list(APPEND test_fwk_multi_thread_util_WRAP fwk_module_get_state)
list(APPEND test_fwk_multi_thread_util_WRAP fwk_mm_calloc)
list(APPEND test_fwk_multi_thread_util_WRAP fwk_interrupt_get_current)
list(APPEND test_fwk_multi_thread_util_WRAP fwk_interrupt_global_disable)
list(APPEND test_fwk_multi_thread_util_WRAP fwk_interrupt_global_enable)
list(APPEND test_fwk_multi_thread_util_WRAP fwk_module_is_valid_element_id)
list(APPEND test_fwk_multi_thread_util_WRAP fwk_module_is_valid_entity_id)
list(APPEND test_fwk_multi_thread_util_WRAP fwk_module_is_valid_event_id)
list(APPEND test_fwk_multi_thread_util_WRAP fwk_module_is_valid_module_id)
list(APPEND test_fwk_multi_thread_util_WRAP fwk_module_is_valid_notification_id)
list(APPEND test_fwk_multi_thread_util_WRAP osKernelInitialize)
list(APPEND test_fwk_multi_thread_util_WRAP osKernelStart)
list(APPEND test_fwk_multi_thread_util_WRAP osThreadFlagsClear)
list(APPEND test_fwk_multi_thread_util_WRAP osThreadFlagsSet)
list(APPEND test_fwk_multi_thread_util_WRAP osThreadFlagsWait)
list(APPEND test_fwk_multi_thread_util_WRAP osThreadNew)

list(APPEND TEST_MODULE_IDX_H test_fwk_module)
set(test_fwk_module_MODULE_IDX_H test_fwk_module_module_idx.h)

list(LENGTH SCP_FWK_TEST_TARGETS SCP_FWK_TEST_MAX)

# Set up common framework source to be built
list(APPEND COMMON_SRC ${FWK_SRC_ROOT}/fwk_arch.c)
list(APPEND COMMON_SRC ${FWK_SRC_ROOT}/fwk_dlist.c)
list(APPEND COMMON_SRC ${FWK_SRC_ROOT}/fwk_id.c)
list(APPEND COMMON_SRC ${FWK_SRC_ROOT}/fwk_io.c)
list(APPEND COMMON_SRC ${FWK_SRC_ROOT}/fwk_interrupt.c)
list(APPEND COMMON_SRC ${FWK_SRC_ROOT}/fwk_log.c)
list(APPEND COMMON_SRC ${FWK_SRC_ROOT}/fwk_mm.c)
list(APPEND COMMON_SRC ${FWK_SRC_ROOT}/fwk_module.c)
list(APPEND COMMON_SRC ${FWK_SRC_ROOT}/fwk_ring.c)
list(APPEND COMMON_SRC ${FWK_SRC_ROOT}/fwk_slist.c)
list(APPEND COMMON_SRC ${FWK_TEST_SRC_ROOT}/fwk_test.c)
list(APPEND COMMON_SRC ${FWK_SRC_ROOT}/fwk_thread_delayed_resp.c)
list(APPEND COMMON_SRC ${FWK_SRC_ROOT}/fwk_time.c)

# CMake internal function enables testing for this directory
enable_testing()

# cmake-lint: disable=E1120
foreach(idx RANGE ${SCP_FWK_TEST_MAX})
    if(idx EQUAL SCP_FWK_TEST_MAX)
        break()
    endif()

    list(GET SCP_FWK_TEST_TARGETS ${idx} TEST_TARGET)

    add_executable(${TEST_TARGET} ${TEST_TARGET}.c)

    target_compile_definitions(
        ${TEST_TARGET}
        PUBLIC "BUILD_VERSION_DESCRIBE_STRING=\"${SCP_FWK_TEST_DESCRIBE}\""
               "BUILD_VERSION_MAJOR=${SCP_FWK_TEST_VERSION_MAJOR}"
               "BUILD_VERSION_MINOR=${SCP_FWK_TEST_VERSION_MINOR}")

    foreach(COMPILE_FLAG IN LISTS EXTRA_COMPILE_FLAGS)
        target_compile_options(${TEST_TARGET} PRIVATE "${COMPILE_FLAG}")
    endforeach()

    target_include_directories(
        ${TEST_TARGET}
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include")

    target_sources(${TEST_TARGET} PRIVATE ${COMMON_SRC})

    # Check whether this test need multi-threading support
    list(FIND MULTITHREADING_ENABLED_TESTS ${TEST_TARGET} MULTI_THREADING)
    if(NOT MULTI_THREADING EQUAL -1)
        target_sources(${TEST_TARGET}
                       PRIVATE ${FWK_SRC_ROOT}/fwk_multi_thread.c)
        target_compile_definitions(${TEST_TARGET}
                                   PUBLIC "BUILD_HAS_MULTITHREADING")
        target_include_directories(
            ${TEST_TARGET}
            PUBLIC ${CMSIS_ROOT}/Include
            PUBLIC ${CMSIS_ROOT}/RTX/Include)
    else()
        target_sources(${TEST_TARGET} PRIVATE ${FWK_SRC_ROOT}/fwk_thread.c)
    endif()

    # Check whether this test need notification support
    list(FIND NOTIFICATION_ENABLED_TEST ${TEST_TARGET} NOTIFICATIONS)
    if(NOT NOTIFICATIONS EQUAL -1)
        target_sources(${TEST_TARGET}
                       PRIVATE ${FWK_SRC_ROOT}/fwk_notification.c)
        target_compile_definitions(${TEST_TARGET}
                                   PUBLIC "BUILD_HAS_NOTIFICATION")
    endif()

    # Check if this test requires any custom module_idx_h file
    list(FIND TEST_MODULE_IDX_H ${TEST_TARGET} MODULE_IDX_H)
    if(NOT MODULE_IDX_H EQUAL -1)
        target_compile_definitions(
            ${TEST_TARGET}
            PRIVATE "FWK_TEST_MODULE_IDX_H=\"${${TEST_TARGET}_MODULE_IDX_H}\"")
    endif()

    # Use linker option to provide wrap functions
    if(${TEST_TARGET}_WRAP)
        foreach(link_option IN LISTS ${TEST_TARGET}_WRAP)
            target_link_options(${TEST_TARGET}
                                PRIVATE "LINKER:-wrap=${link_option}")
        endforeach()
    endif()

    add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})

endforeach()