#
# Arm SCP/MCP Software
# Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include $(BS_DIR)/defs.mk
CFLAGS += $(addprefix -D,$(DEFINES)) -DBUILD_HAS_NOTIFICATION

#
# List of test c and dependencies
#

COMMON_SRC := fwk_assert.c fwk_interrupt.c

TESTS += test_fwk_macros
test_fwk_macros_SRC := test_fwk_macros.c fwk_test.c

TESTS += test_fwk_list_get
test_fwk_list_get_SRC := test_fwk_list_get.c fwk_test.c fwk_dlist.c \
    fwk_slist.c

TESTS += test_fwk_list_init
test_fwk_list_init_SRC := test_fwk_list_init.c fwk_test.c fwk_dlist.c \
	fwk_slist.c

TESTS += test_fwk_list_head
test_fwk_list_head_SRC := test_fwk_list_head.c fwk_test.c fwk_dlist.c \
    fwk_slist.c

TESTS += test_fwk_list_push
test_fwk_list_push_SRC := test_fwk_list_push.c fwk_test.c fwk_dlist.c \
    fwk_slist.c

TESTS += test_fwk_list_pop
test_fwk_list_pop_SRC := test_fwk_list_pop.c fwk_test.c fwk_dlist.c \
    fwk_slist.c

TESTS += test_fwk_list_next
test_fwk_list_next_SRC := test_fwk_list_next.c fwk_test.c fwk_dlist.c \
    fwk_slist.c

TESTS += test_fwk_list_remove
test_fwk_list_remove_SRC := test_fwk_list_remove.c fwk_test.c fwk_dlist.c \
    fwk_slist.c

TESTS += test_fwk_list_empty
test_fwk_list_empty_SRC := test_fwk_list_empty.c fwk_test.c fwk_dlist.c \
    fwk_slist.c

TESTS += test_fwk_list_insert
test_fwk_list_insert_SRC := test_fwk_list_insert.c fwk_test.c \
    fwk_dlist.c fwk_slist.c

TESTS += test_fwk_list_contains
test_fwk_list_contains_SRC := test_fwk_list_contains.c fwk_test.c \
    fwk_dlist.c fwk_slist.c

TESTS += test_fwk_mm
test_fwk_mm_SRC := test_fwk_mm.c fwk_mm.c fwk_test.c

TESTS += test_fwk_arch
test_fwk_arch_SRC := test_fwk_arch.c fwk_arch.c fwk_test.c
test_fwk_arch_WRAP := fwk_interrupt_init

TESTS += test_fwk_interrupt
test_fwk_interrupt_SRC := test_fwk_interrupt.c fwk_test.c

TESTS += test_version
test_version_SRC := test_version.c fwk_test.c

TESTS += test_fwk_id_build
test_fwk_id_build_SRC := test_fwk_id_build.c fwk_test.c fwk_id.c

TESTS += test_fwk_id_type
test_fwk_id_type_SRC := test_fwk_id_type.c fwk_test.c fwk_id.c

TESTS += test_fwk_id_format
test_fwk_id_format_SRC := test_fwk_id_format.c fwk_test.c fwk_id.c

TESTS += test_fwk_id_get_idx
test_fwk_id_get_idx_SRC := test_fwk_id_get_idx.c fwk_test.c fwk_id.c

TESTS += test_fwk_id_equality
test_fwk_id_equality_SRC := test_fwk_id_equality.c fwk_test.c fwk_id.c

TESTS += test_fwk_module
test_fwk_module_SRC := test_fwk_module.c fwk_module.c fwk_test.c fwk_id.c \
    fwk_slist.c fwk_dlist.c
test_fwk_module_WRAP := fwk_mm_calloc __fwk_thread_init __fwk_thread_run \
__fwk_notification_init

TESTS += test_fwk_thread
test_fwk_thread_SRC := test_fwk_thread.c fwk_thread.c fwk_test.c fwk_slist.c \
    fwk_id.c fwk_thread_delayed_resp.c
test_fwk_thread_WRAP := fwk_mm_calloc fwk_module_is_valid_entity_id \
    fwk_module_is_valid_event_id __fwk_slist_push_tail __fwk_module_get_ctx \
    fwk_interrupt_global_enable fwk_interrupt_global_disable \
    fwk_interrupt_get_current fwk_module_is_valid_notification_id \
    __fwk_module_get_element_ctx

TESTS += test_fwk_notification
test_fwk_notification_SRC := test_fwk_notification.c fwk_notification.c \
    fwk_test.c fwk_dlist.c fwk_slist.c fwk_id.c
test_fwk_notification_WRAP := fwk_mm_calloc fwk_module_is_valid_entity_id \
    fwk_module_is_valid_notification_id __fwk_module_get_ctx \
    __fwk_module_get_element_ctx fwk_interrupt_global_enable \
    fwk_interrupt_global_disable fwk_interrupt_get_current \
    __fwk_thread_put_notification __fwk_thread_get_current_event

# Multi-thread tests
TESTS += test_fwk_multi_thread_init
test_fwk_multi_thread_init_SRC := test_fwk_multi_thread_init.c \
    fwk_multi_thread.c fwk_test.c fwk_slist.c fwk_id.c fwk_thread_delayed_resp.c
test_fwk_multi_thread_init_WRAP := fwk_mm_calloc fwk_interrupt_get_current \
    osThreadFlagsWait osThreadFlagsSet osThreadNew __fwk_module_get_ctx \
    fwk_interrupt_global_disable fwk_interrupt_global_enable \
    fwk_module_is_valid_element_id __fwk_module_get_element_ctx \
    __fwk_module_get_element_ctx  __fwk_module_get_state \
    fwk_module_is_valid_module_id osKernelStart osKernelInitialize \
    fwk_module_is_valid_entity_id fwk_module_is_valid_event_id \
    fwk_module_is_valid_notification_id

TESTS += test_fwk_multi_thread_create
test_fwk_multi_thread_create_SRC := test_fwk_multi_thread_create.c \
    fwk_multi_thread.c fwk_test.c fwk_slist.c fwk_id.c fwk_thread_delayed_resp.c
test_fwk_multi_thread_create_WRAP := fwk_mm_calloc fwk_interrupt_get_current \
    osThreadFlagsWait osThreadFlagsSet osThreadNew __fwk_module_get_ctx \
    fwk_interrupt_global_disable fwk_interrupt_global_enable \
    fwk_module_is_valid_element_id __fwk_module_get_element_ctx \
    __fwk_module_get_element_ctx __fwk_module_get_state \
    fwk_module_is_valid_module_id osKernelStart osKernelInitialize \
    fwk_module_is_valid_entity_id fwk_module_is_valid_event_id \
    fwk_module_is_valid_notification_id

TESTS += test_fwk_multi_thread_common_thread
test_fwk_multi_thread_common_thread_SRC := fwk_multi_thread.c fwk_test.c \
    test_fwk_multi_thread_common_thread.c fwk_slist.c fwk_id.c \
    fwk_thread_delayed_resp.c
test_fwk_multi_thread_common_thread_WRAP := fwk_mm_calloc \
    osThreadFlagsWait osThreadFlagsSet osThreadNew __fwk_module_get_ctx \
    fwk_interrupt_global_disable fwk_interrupt_global_enable \
    fwk_module_is_valid_element_id __fwk_module_get_element_ctx \
    __fwk_module_get_state fwk_module_is_valid_module_id osKernelStart \
    osKernelInitialize fwk_module_is_valid_entity_id \
    fwk_module_is_valid_event_id fwk_module_is_valid_notification_id \
    fwk_interrupt_get_current

TESTS += test_fwk_multi_thread_put_event
test_fwk_multi_thread_put_event_SRC := test_fwk_multi_thread_put_event.c \
    fwk_multi_thread.c fwk_test.c fwk_slist.c fwk_id.c fwk_thread_delayed_resp.c
test_fwk_multi_thread_put_event_WRAP := fwk_mm_calloc __fwk_module_get_state \
    fwk_interrupt_get_current osThreadFlagsWait osThreadFlagsSet osThreadNew \
    fwk_interrupt_global_disable fwk_interrupt_global_enable \
    fwk_module_is_valid_element_id __fwk_module_get_element_ctx \
    __fwk_module_get_element_ctx __fwk_module_get_ctx \
    fwk_module_is_valid_module_id osKernelStart osKernelInitialize \
    fwk_module_is_valid_entity_id fwk_module_is_valid_event_id \
    fwk_module_is_valid_notification_id

TESTS += test_fwk_multi_thread_util
test_fwk_multi_thread_util_SRC := test_fwk_multi_thread_util.c \
    fwk_multi_thread.c fwk_test.c fwk_slist.c fwk_id.c fwk_thread_delayed_resp.c
test_fwk_multi_thread_util_WRAP := fwk_mm_calloc __fwk_module_get_state \
    fwk_interrupt_get_current osThreadFlagsWait osThreadFlagsSet osThreadNew \
    fwk_interrupt_global_disable fwk_interrupt_global_enable \
    fwk_module_is_valid_element_id __fwk_module_get_element_ctx \
    __fwk_module_get_element_ctx __fwk_module_get_ctx \
    fwk_module_is_valid_module_id osKernelStart osKernelInitialize \
    fwk_module_is_valid_entity_id fwk_module_is_valid_event_id \
    fwk_module_is_valid_notification_id

TESTS += test_fwk_math
test_fwk_math_SRC := test_fwk_math.c fwk_test.c

include $(BS_DIR)/test.mk
