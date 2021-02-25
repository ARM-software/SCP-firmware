/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/fwk_context.h>
#include <internal/fwk_core.h>
#include <internal/fwk_module.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_list.h>
#include <fwk_macros.h>
#include <fwk_slist.h>
#include <fwk_status.h>
#include <fwk_test.h>

#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>

/* Mock functions */
static void * fwk_mm_calloc_val;
static size_t fwk_mm_calloc_num;
static size_t fwk_mm_calloc_size;
static int fwk_mm_calloc_return_val;
void *__wrap_fwk_mm_calloc(size_t num, size_t size)
{
    if (fwk_mm_calloc_return_val) {
        fwk_mm_calloc_num = num;
        fwk_mm_calloc_size = size;
        fwk_mm_calloc_val = (void *)calloc(num, size);
        return fwk_mm_calloc_val;
    }

    fwk_mm_calloc_num = 0;
    fwk_mm_calloc_size = 0;

    return NULL;
}

static struct fwk_module_context fake_module_ctx;
static struct fwk_dlist fake_module_dlist_table[4];
struct fwk_module_context *__wrap_fwk_module_get_ctx(fwk_id_t id)
{
    fake_module_ctx.subscription_dlist_table = fake_module_dlist_table;
    return &fake_module_ctx;
}

static struct fwk_element_ctx fake_element_ctx;
static struct fwk_dlist fake_element_dlist_table[4];
struct fwk_element_ctx *__wrap_fwk_module_get_element_ctx(fwk_id_t id)
{
    fake_element_ctx.subscription_dlist_table = fake_element_dlist_table;
    return &fake_element_ctx;
}

static bool is_valid_entity_id_return_val;
bool __wrap_fwk_module_is_valid_entity_id(fwk_id_t id)
{
    return is_valid_entity_id_return_val;
}

static bool is_valid_notification_id_return_val;
bool __wrap_fwk_module_is_valid_notification_id(fwk_id_t id)
{
    return is_valid_notification_id_return_val;
}

int __wrap_fwk_interrupt_global_enable(void)
{
    return FWK_SUCCESS;
}

int __wrap_fwk_interrupt_global_disable(void)
{
    return FWK_SUCCESS;
}

static bool interrupt_get_current_return_val;
bool __wrap_fwk_is_interrupt_context(void)
{
    return interrupt_get_current_return_val;
}

static struct fwk_event notification_event_table[4];
static unsigned int notification_event_count;
int __wrap___fwk_put_notification(struct fwk_event *event)
{
    assert(notification_event_count < FWK_ARRAY_SIZE(notification_event_table));

    notification_event_table[notification_event_count++] = *event;

    return FWK_SUCCESS;
}

static struct fwk_event *get_current_event_return_val;
const struct fwk_event *__wrap___fwk_get_current_event(void)
{
    return get_current_event_return_val;
}

static void test_case_setup(void)
{
    unsigned int i;

    is_valid_entity_id_return_val = true;
    is_valid_notification_id_return_val = true;
    interrupt_get_current_return_val = false;
    fwk_mm_calloc_return_val = true;
    get_current_event_return_val = NULL;
    notification_event_count = 0;

    for (i = 0; i < FWK_ARRAY_SIZE(fake_module_dlist_table); i++)
        fwk_list_init(&fake_module_dlist_table[i]);

    for (i = 0; i < FWK_ARRAY_SIZE(fake_element_dlist_table); i++)
        fwk_list_init(&fake_element_dlist_table[i]);
}

static void test_case_teardown(void)
{
    __fwk_notification_reset();
}

static void test_fwk_notification_subscribe(void)
{
    int result;

    /* Call from an ISR */
    interrupt_get_current_return_val = true;
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x3),
                                        FWK_ID_ELEMENT(0x2, 0x9),
                                        FWK_ID_MODULE(0x4));
    assert(result == FWK_E_HANDLER);
    interrupt_get_current_return_val = false;

    /* Invalid entity ID */
    is_valid_entity_id_return_val = false;
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x3),
                                        FWK_ID_ELEMENT(0x2, 0x9),
                                        FWK_ID_MODULE(0x4));
    assert(result == FWK_E_PARAM);
    is_valid_entity_id_return_val = true;

    /* Invalid notification ID */
    is_valid_notification_id_return_val = false;
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x3),
                                        FWK_ID_ELEMENT(0x2, 0x9),
                                        FWK_ID_MODULE(0x4));
    assert(result == FWK_E_PARAM);
    is_valid_notification_id_return_val = true;

    /* Incompatible source and event identifier */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x3, 0x3),
                                        FWK_ID_ELEMENT(0x2, 0x9),
                                        FWK_ID_MODULE(0x4));
    assert(result == FWK_E_PARAM);

    /* Subscribe successfully to an element notification */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x3),
                                        FWK_ID_ELEMENT(0x2, 0x9),
                                        FWK_ID_MODULE(0x4));
    assert(result == FWK_SUCCESS);

    /* Try to subscribe to the same element notification */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x3),
                                        FWK_ID_ELEMENT(0x2, 0x9),
                                        FWK_ID_MODULE(0x4));
    assert(result == FWK_E_STATE);

    /* Subscribe successfully to a module notification */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x1),
                                        FWK_ID_MODULE(0x2),
                                        FWK_ID_ELEMENT(0x4, 0x15));
    assert(result == FWK_SUCCESS);

    /* Try to subscribe to the same module notification */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x1),
                                        FWK_ID_MODULE(0x2),
                                        FWK_ID_ELEMENT(0x4, 0x15));
    assert(result == FWK_E_STATE);
}

static void test_fwk_notification_unsubscribe(void)
{
    int result;

    /* Call from an ISR */
    interrupt_get_current_return_val = true;
    result = fwk_notification_unsubscribe(FWK_ID_NOTIFICATION(0x2, 0x3),
                                          FWK_ID_ELEMENT(0x2, 0x9),
                                          FWK_ID_MODULE(0x4));
    assert(result == FWK_E_HANDLER);
    interrupt_get_current_return_val = false;

    /* Invalid entity ID */
    is_valid_entity_id_return_val = false;
    result = fwk_notification_unsubscribe(FWK_ID_NOTIFICATION(0x2, 0x3),
                                          FWK_ID_ELEMENT(0x2, 0x9),
                                          FWK_ID_MODULE(0x4));
    assert(result == FWK_E_PARAM);
    is_valid_entity_id_return_val = true;

    /* Invalid notification ID */
    is_valid_notification_id_return_val = false;
    result = fwk_notification_unsubscribe(FWK_ID_NOTIFICATION(0x2, 0x3),
                                          FWK_ID_ELEMENT(0x2, 0x9),
                                          FWK_ID_MODULE(0x4));
    assert(result == FWK_E_PARAM);
    is_valid_notification_id_return_val = true;

    /* Incompatible source and event identifier */
    result = fwk_notification_unsubscribe(FWK_ID_NOTIFICATION(0x3, 0x3),
                                          FWK_ID_ELEMENT(0x2, 0x9),
                                          FWK_ID_MODULE(0x4));
    assert(result == FWK_E_PARAM);

    /* Subscribe to an element notification */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x1),
                                        FWK_ID_ELEMENT(0x2, 0x9),
                                        FWK_ID_MODULE(0x4));
    assert(result == FWK_SUCCESS);

    /* Subscribe to the same element notification for another target */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x1),
                                        FWK_ID_ELEMENT(0x2, 0x9),
                                        FWK_ID_ELEMENT(0x6, 0x1));
    assert(result == FWK_SUCCESS);

    /* Subscribe to a module notification */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x1),
                                        FWK_ID_MODULE(0x2),
                                        FWK_ID_ELEMENT(0x4, 0x15));
    assert(result == FWK_SUCCESS);

    /* Unsubscribe to a notification that has not been subscribed to */
    result = fwk_notification_unsubscribe(FWK_ID_NOTIFICATION(0x2, 0x1),
                                          FWK_ID_ELEMENT(0x2, 0x9),
                                          FWK_ID_MODULE(0x6));
    assert(result == FWK_E_STATE);

    /*
     * Unsubscribe to the element notification that has been subscribed to
     * first
     */
    result = fwk_notification_unsubscribe(FWK_ID_NOTIFICATION(0x2, 0x1),
                                          FWK_ID_ELEMENT(0x2, 0x9),
                                          FWK_ID_MODULE(0x4));
    assert(result == FWK_SUCCESS);

    /* Subscribe to another element notification, the subscription freed in the
       previous step is re-used */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x3),
                                        FWK_ID_MODULE(0x2),
                                        FWK_ID_ELEMENT(0xC, 0xC));
    assert(result == FWK_SUCCESS);
}

static void test_fwk_notification_notify(void)
{
    int result;
    struct fwk_event notification_event, current_event;
    unsigned int count;

    /* Call from an ISR, invalid source identifier */
    interrupt_get_current_return_val = true;
    is_valid_entity_id_return_val = false;
    result = fwk_notification_notify(&notification_event, &count);
    assert(result == FWK_E_PARAM);
    interrupt_get_current_return_val = false;
    is_valid_entity_id_return_val = true;

    /* Current event, incompatible notification and source identifier. */
    current_event.target_id = FWK_ID_ELEMENT(0x2, 0x9);
    notification_event.source_id = FWK_ID_ELEMENT(0x3, 0x9);
    notification_event.id = FWK_ID_NOTIFICATION(0x2, 0x1);
    get_current_event_return_val = &current_event;
    result = fwk_notification_notify(&notification_event, &count);
    assert(result == FWK_E_PARAM);
    get_current_event_return_val = NULL;

    /* No current event, invalid notification identifier. */
    is_valid_notification_id_return_val = false;
    result = fwk_notification_notify(&notification_event, &count);
    assert(result == FWK_E_PARAM);
    is_valid_notification_id_return_val = true;

    /* No current event, incompatible notification and
       source identifier. */
    notification_event.source_id = FWK_ID_ELEMENT(0x2, 0x9);
    notification_event.id = FWK_ID_NOTIFICATION(0x1, 0x1);
    result = fwk_notification_notify(&notification_event, &count);
    assert(result == FWK_E_PARAM);

    /* Subscribe to an element notification */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x1),
                                        FWK_ID_ELEMENT(0x2, 0x9),
                                        FWK_ID_MODULE(0x4));
    assert(result == FWK_SUCCESS);

    /* Subscribe to the same element notification for another target */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x1),
                                        FWK_ID_ELEMENT(0x2, 0x9),
                                        FWK_ID_ELEMENT(0x6, 0x1));
    assert(result == FWK_SUCCESS);

    /* Subscribe to a module notification */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x1),
                                        FWK_ID_MODULE(0x2),
                                        FWK_ID_ELEMENT(0x4, 0x15));
    assert(result == FWK_SUCCESS);

    /* Send the notifications FWK_ID_NOTIFICATION(0x2, 0x1) from
       FWK_ID_ELEMENT(0x2, 0x9) */
    notification_event.source_id = FWK_ID_ELEMENT(0x2, 0x9);
    notification_event.id = FWK_ID_NOTIFICATION(0x2, 0x1);
    result = fwk_notification_notify(&notification_event, &count);
    assert(result == FWK_SUCCESS);
    assert(count == 2);
    assert(notification_event_count == 2);
    assert(fwk_id_is_equal(notification_event_table[0].source_id,
                           FWK_ID_ELEMENT(0x2, 0x9)));
    assert(fwk_id_is_equal(notification_event_table[0].target_id,
                           FWK_ID_MODULE(0x4)));
    assert(fwk_id_is_equal(notification_event_table[0].id,
                           FWK_ID_NOTIFICATION(0x2, 0x1)));
    assert(fwk_id_is_equal(notification_event_table[1].source_id,
                           FWK_ID_ELEMENT(0x2, 0x9)));
    assert(fwk_id_is_equal(notification_event_table[1].target_id,
                           FWK_ID_ELEMENT(0x6, 0x1)));
    assert(fwk_id_is_equal(notification_event_table[1].id,
                           FWK_ID_NOTIFICATION(0x2, 0x1)));
    notification_event_count = 0;

    /* Send the notification FWK_ID_NOTIFICATION(0x2, 0x1) from
       FWK_ID_MODULE(0x2) */

    current_event.target_id = FWK_ID_MODULE(0x2);
    get_current_event_return_val = &current_event;
    notification_event.id = FWK_ID_NOTIFICATION(0x2, 0x1);
    is_valid_entity_id_return_val = false;
    result = fwk_notification_notify(&notification_event, &count);
    assert(result == FWK_SUCCESS);
    assert(count == 1);
    assert(notification_event_count == 1);
    assert(fwk_id_is_equal(notification_event_table[0].source_id,
                           FWK_ID_MODULE(0x2)));
    assert(fwk_id_is_equal(notification_event_table[0].target_id,
                           FWK_ID_ELEMENT(0x4, 0x15)));
    assert(fwk_id_is_equal(notification_event_table[0].id,
                           FWK_ID_NOTIFICATION(0x2, 0x1)));
    get_current_event_return_val = NULL;
    notification_event_count = 0;
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_fwk_notification_subscribe),
    FWK_TEST_CASE(test_fwk_notification_unsubscribe),
    FWK_TEST_CASE(test_fwk_notification_notify)
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_core",
    .test_case_setup = test_case_setup,
    .test_case_teardown = test_case_teardown,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
