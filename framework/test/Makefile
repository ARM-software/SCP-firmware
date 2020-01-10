#
# Arm SCP/MCP Software
# Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include $(BS_DIR)/defs.mk

TESTS := test_fwk_arch
TESTS += test_fwk_id_build
TESTS += test_fwk_id_equality
TESTS += test_fwk_id_format
TESTS += test_fwk_id_get_idx
TESTS += test_fwk_id_type
TESTS += test_fwk_interrupt
TESTS += test_fwk_list_contains
TESTS += test_fwk_list_empty
TESTS += test_fwk_list_get
TESTS += test_fwk_list_head
TESTS += test_fwk_list_init
TESTS += test_fwk_list_insert
TESTS += test_fwk_list_next
TESTS += test_fwk_list_pop
TESTS += test_fwk_list_push
TESTS += test_fwk_list_remove
TESTS += test_fwk_macros
TESTS += test_fwk_math
TESTS += test_fwk_mm
TESTS += test_fwk_module
TESTS += test_fwk_multi_thread_common_thread
TESTS += test_fwk_multi_thread_create
TESTS += test_fwk_multi_thread_init
TESTS += test_fwk_multi_thread_put_event
TESTS += test_fwk_multi_thread_util
TESTS += test_fwk_notification
TESTS += test_fwk_ring
TESTS += test_fwk_ring_init
TESTS += test_fwk_thread

COMMON_SRC := fwk_arch.c
COMMON_SRC += fwk_assert.c
COMMON_SRC += fwk_dlist.c
COMMON_SRC += fwk_id.c
COMMON_SRC += fwk_interrupt.c
COMMON_SRC += fwk_log.c
COMMON_SRC += fwk_mm.c
COMMON_SRC += fwk_module.c
COMMON_SRC += fwk_ring.c
COMMON_SRC += fwk_slist.c
COMMON_SRC += fwk_test.c
COMMON_SRC += fwk_thread_delayed_resp.c

test_fwk_arch_SRC += fwk_thread.c
test_fwk_id_build_SRC += fwk_thread.c
test_fwk_id_equality_SRC += fwk_thread.c
test_fwk_id_format_SRC += fwk_thread.c
test_fwk_id_get_idx_SRC += fwk_thread.c
test_fwk_id_type_SRC += fwk_thread.c
test_fwk_interrupt_SRC += fwk_thread.c
test_fwk_list_contains_SRC += fwk_thread.c
test_fwk_list_empty_SRC += fwk_thread.c
test_fwk_list_get_SRC += fwk_thread.c
test_fwk_list_head_SRC += fwk_thread.c
test_fwk_list_init_SRC += fwk_thread.c
test_fwk_list_insert_SRC += fwk_thread.c
test_fwk_list_next_SRC += fwk_thread.c
test_fwk_list_pop_SRC += fwk_thread.c
test_fwk_list_push_SRC += fwk_thread.c
test_fwk_list_remove_SRC += fwk_thread.c
test_fwk_macros_SRC += fwk_thread.c
test_fwk_math_SRC += fwk_thread.c
test_fwk_mm_SRC += fwk_thread.c
test_fwk_module_SRC += fwk_thread.c
test_fwk_multi_thread_common_thread_SRC += fwk_multi_thread.c
test_fwk_multi_thread_create_SRC += fwk_multi_thread.c
test_fwk_multi_thread_init_SRC += fwk_multi_thread.c
test_fwk_multi_thread_put_event_SRC += fwk_multi_thread.c
test_fwk_multi_thread_util_SRC += fwk_multi_thread.c
test_fwk_notification_SRC += fwk_thread.c
test_fwk_ring_SRC += fwk_thread.c
test_fwk_ring_init_SRC += fwk_thread.c
test_fwk_thread_SRC += fwk_thread.c

test_fwk_module_SRC += fwk_notification.c
test_fwk_notification_SRC += fwk_notification.c
test_fwk_thread_SRC += fwk_notification.c

test_fwk_module_CFLAGS += -DBUILD_HAS_NOTIFICATION
test_fwk_notification_CFLAGS += -DBUILD_HAS_NOTIFICATION
test_fwk_thread_CFLAGS += -DBUILD_HAS_NOTIFICATION

test_fwk_arch_WRAP := __fwk_module_init
test_fwk_arch_WRAP += fwk_interrupt_init
test_fwk_arch_WRAP += fwk_mm_init

test_fwk_module_WRAP := __fwk_notification_init
test_fwk_module_WRAP += __fwk_thread_init
test_fwk_module_WRAP += __fwk_thread_run
test_fwk_module_WRAP += fwk_mm_calloc

test_fwk_thread_WRAP := __fwk_module_get_ctx
test_fwk_thread_WRAP += __fwk_module_get_element_ctx
test_fwk_thread_WRAP += __fwk_slist_push_tail
test_fwk_thread_WRAP += fwk_interrupt_get_current
test_fwk_thread_WRAP += fwk_interrupt_global_disable
test_fwk_thread_WRAP += fwk_interrupt_global_enable
test_fwk_thread_WRAP += fwk_mm_calloc
test_fwk_thread_WRAP += fwk_module_is_valid_entity_id
test_fwk_thread_WRAP += fwk_module_is_valid_event_id
test_fwk_thread_WRAP += fwk_module_is_valid_notification_id

test_fwk_notification_WRAP := __fwk_module_get_ctx
test_fwk_notification_WRAP += __fwk_module_get_element_ctx
test_fwk_notification_WRAP += __fwk_thread_get_current_event
test_fwk_notification_WRAP += __fwk_thread_put_notification
test_fwk_notification_WRAP += fwk_interrupt_get_current
test_fwk_notification_WRAP += fwk_interrupt_global_disable
test_fwk_notification_WRAP += fwk_interrupt_global_enable
test_fwk_notification_WRAP += fwk_mm_calloc
test_fwk_notification_WRAP += fwk_module_is_valid_entity_id
test_fwk_notification_WRAP += fwk_module_is_valid_notification_id

test_fwk_multi_thread_init_WRAP := __fwk_module_get_ctx
test_fwk_multi_thread_init_WRAP += __fwk_module_get_element_ctx
test_fwk_multi_thread_init_WRAP += __fwk_module_get_element_ctx
test_fwk_multi_thread_init_WRAP += __fwk_module_get_state
test_fwk_multi_thread_init_WRAP += fwk_interrupt_get_current
test_fwk_multi_thread_init_WRAP += fwk_interrupt_global_disable
test_fwk_multi_thread_init_WRAP += fwk_interrupt_global_enable
test_fwk_multi_thread_init_WRAP += fwk_mm_calloc
test_fwk_multi_thread_init_WRAP += fwk_module_is_valid_element_id
test_fwk_multi_thread_init_WRAP += fwk_module_is_valid_entity_id
test_fwk_multi_thread_init_WRAP += fwk_module_is_valid_event_id
test_fwk_multi_thread_init_WRAP += fwk_module_is_valid_module_id
test_fwk_multi_thread_init_WRAP += fwk_module_is_valid_notification_id
test_fwk_multi_thread_init_WRAP += osKernelInitialize
test_fwk_multi_thread_init_WRAP += osKernelStart
test_fwk_multi_thread_init_WRAP += osThreadFlagsClear
test_fwk_multi_thread_init_WRAP += osThreadFlagsSet
test_fwk_multi_thread_init_WRAP += osThreadFlagsWait
test_fwk_multi_thread_init_WRAP += osThreadNew

test_fwk_multi_thread_create_WRAP := __fwk_module_get_ctx
test_fwk_multi_thread_create_WRAP += __fwk_module_get_element_ctx
test_fwk_multi_thread_create_WRAP += __fwk_module_get_element_ctx
test_fwk_multi_thread_create_WRAP += __fwk_module_get_state
test_fwk_multi_thread_create_WRAP += fwk_interrupt_get_current
test_fwk_multi_thread_create_WRAP += fwk_interrupt_global_disable
test_fwk_multi_thread_create_WRAP += fwk_interrupt_global_enable
test_fwk_multi_thread_create_WRAP += fwk_mm_calloc
test_fwk_multi_thread_create_WRAP += fwk_module_is_valid_element_id
test_fwk_multi_thread_create_WRAP += fwk_module_is_valid_entity_id
test_fwk_multi_thread_create_WRAP += fwk_module_is_valid_event_id
test_fwk_multi_thread_create_WRAP += fwk_module_is_valid_module_id
test_fwk_multi_thread_create_WRAP += fwk_module_is_valid_notification_id
test_fwk_multi_thread_create_WRAP += osKernelInitialize
test_fwk_multi_thread_create_WRAP += osKernelStart
test_fwk_multi_thread_create_WRAP += osThreadFlagsClear
test_fwk_multi_thread_create_WRAP += osThreadFlagsSet
test_fwk_multi_thread_create_WRAP += osThreadFlagsWait
test_fwk_multi_thread_create_WRAP += osThreadNew

test_fwk_multi_thread_common_thread_WRAP := __fwk_module_get_ctx
test_fwk_multi_thread_common_thread_WRAP += __fwk_module_get_element_ctx
test_fwk_multi_thread_common_thread_WRAP += __fwk_module_get_state
test_fwk_multi_thread_common_thread_WRAP += fwk_interrupt_get_current
test_fwk_multi_thread_common_thread_WRAP += fwk_interrupt_global_disable
test_fwk_multi_thread_common_thread_WRAP += fwk_interrupt_global_enable
test_fwk_multi_thread_common_thread_WRAP += fwk_mm_calloc
test_fwk_multi_thread_common_thread_WRAP += fwk_module_is_valid_element_id
test_fwk_multi_thread_common_thread_WRAP += fwk_module_is_valid_entity_id
test_fwk_multi_thread_common_thread_WRAP += fwk_module_is_valid_event_id
test_fwk_multi_thread_common_thread_WRAP += fwk_module_is_valid_module_id
test_fwk_multi_thread_common_thread_WRAP += fwk_module_is_valid_notification_id
test_fwk_multi_thread_common_thread_WRAP += osKernelInitialize
test_fwk_multi_thread_common_thread_WRAP += osKernelStart
test_fwk_multi_thread_common_thread_WRAP += osThreadFlagsClear
test_fwk_multi_thread_common_thread_WRAP += osThreadFlagsSet
test_fwk_multi_thread_common_thread_WRAP += osThreadFlagsWait
test_fwk_multi_thread_common_thread_WRAP += osThreadNew

test_fwk_multi_thread_put_event_WRAP := __fwk_module_get_ctx
test_fwk_multi_thread_put_event_WRAP += __fwk_module_get_element_ctx
test_fwk_multi_thread_put_event_WRAP += __fwk_module_get_element_ctx
test_fwk_multi_thread_put_event_WRAP += __fwk_module_get_state
test_fwk_multi_thread_put_event_WRAP += fwk_interrupt_get_current
test_fwk_multi_thread_put_event_WRAP += fwk_interrupt_global_disable
test_fwk_multi_thread_put_event_WRAP += fwk_interrupt_global_enable
test_fwk_multi_thread_put_event_WRAP += fwk_mm_calloc
test_fwk_multi_thread_put_event_WRAP += fwk_module_is_valid_element_id
test_fwk_multi_thread_put_event_WRAP += fwk_module_is_valid_entity_id
test_fwk_multi_thread_put_event_WRAP += fwk_module_is_valid_event_id
test_fwk_multi_thread_put_event_WRAP += fwk_module_is_valid_module_id
test_fwk_multi_thread_put_event_WRAP += fwk_module_is_valid_notification_id
test_fwk_multi_thread_put_event_WRAP += osKernelInitialize
test_fwk_multi_thread_put_event_WRAP += osKernelStart
test_fwk_multi_thread_put_event_WRAP += osThreadFlagsClear
test_fwk_multi_thread_put_event_WRAP += osThreadFlagsSet
test_fwk_multi_thread_put_event_WRAP += osThreadFlagsWait
test_fwk_multi_thread_put_event_WRAP += osThreadNew

test_fwk_multi_thread_util_WRAP := __fwk_module_get_ctx
test_fwk_multi_thread_util_WRAP += __fwk_module_get_element_ctx
test_fwk_multi_thread_util_WRAP += __fwk_module_get_element_ctx
test_fwk_multi_thread_util_WRAP += __fwk_module_get_state
test_fwk_multi_thread_util_WRAP += fwk_interrupt_get_current
test_fwk_multi_thread_util_WRAP += fwk_interrupt_global_disable
test_fwk_multi_thread_util_WRAP += fwk_interrupt_global_enable
test_fwk_multi_thread_util_WRAP += fwk_mm_calloc
test_fwk_multi_thread_util_WRAP += fwk_module_is_valid_element_id
test_fwk_multi_thread_util_WRAP += fwk_module_is_valid_entity_id
test_fwk_multi_thread_util_WRAP += fwk_module_is_valid_event_id
test_fwk_multi_thread_util_WRAP += fwk_module_is_valid_module_id
test_fwk_multi_thread_util_WRAP += fwk_module_is_valid_notification_id
test_fwk_multi_thread_util_WRAP += osKernelInitialize
test_fwk_multi_thread_util_WRAP += osKernelStart
test_fwk_multi_thread_util_WRAP += osThreadFlagsClear
test_fwk_multi_thread_util_WRAP += osThreadFlagsSet
test_fwk_multi_thread_util_WRAP += osThreadFlagsWait
test_fwk_multi_thread_util_WRAP += osThreadNew

$(foreach test, $(TESTS), \
    $(if $(filter fwk_multi_thread.c, $($(test)_SRC)), \
		$(eval $(test)_CFLAGS += -DBUILD_HAS_MULTITHREADING)))

include $(BS_DIR)/test.mk
