/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/fwk_module.h>
#include <internal/fwk_single_thread.h>
#include <internal/fwk_thread.h>

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

static struct fwk_module_ctx fake_module_ctx;
static struct fwk_dlist fake_module_dlist_table[4];
struct fwk_module_ctx *__wrap___fwk_module_get_ctx(fwk_id_t id)
{
    fake_module_ctx.subscription_dlist_table = fake_module_dlist_table;
    return &fake_module_ctx;
}

static struct fwk_element_ctx fake_element_ctx;
static struct fwk_dlist fake_element_dlist_table[4];
struct fwk_element_ctx *__wrap___fwk_module_get_element_ctx(fwk_id_t id)
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

static int interrupt_get_current_return_val;
int __wrap_fwk_interrupt_get_current(unsigned int *interrupt)
{
    return interrupt_get_current_return_val;
}

static struct fwk_event notification_event_table[4];
static unsigned int notification_event_count;
int __wrap___fwk_thread_put_notification(struct fwk_event *event)
{
    assert(notification_event_count < FWK_ARRAY_SIZE(notification_event_table));

    notification_event_table[notification_event_count++] = *event;

    return FWK_SUCCESS;
}

static struct fwk_event *get_current_event_return_val;
const struct fwk_event *__wrap___fwk_thread_get_current_event(void)
{
    return get_current_event_return_val;
}

static void test_case_setup(void)
{
    unsigned int i;

    is_valid_entity_id_return_val = true;
    is_valid_notification_id_return_val = true;
    interrupt_get_current_return_val = FWK_E_STATE;
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

static void test___fwk_notification_init(void)
{
    int result;
    size_t notification_count = 4;

    fwk_mm_calloc_return_val = true;

    /* Insert 2 events in the list */
    result = __fwk_notification_init(notification_count);
    assert(result == FWK_SUCCESS);
    assert(fwk_mm_calloc_num == notification_count);
    assert(fwk_mm_calloc_size ==
           sizeof(struct __fwk_notification_subscription));
}

static void test_fwk_notification_subscribe(void)
{
    int result;
    struct __fwk_notification_subscription *subscription1, *subscription2;

    /* Notifications not initialized */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x3),
                                        FWK_ID_ELEMENT(0x2, 0x9),
                                        FWK_ID_MODULE(0x4));
    assert(result == FWK_E_INIT);

    /* Notifications initialization */
    result = __fwk_notification_init(4);
    assert(result == FWK_SUCCESS);
    subscription1 = fwk_mm_calloc_val;
    subscription2 = subscription1 + 1;

    /* Call from an ISR */
    interrupt_get_current_return_val = FWK_SUCCESS;
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x3),
                                        FWK_ID_ELEMENT(0x2, 0x9),
                                        FWK_ID_MODULE(0x4));
    assert(result == FWK_E_HANDLER);
    interrupt_get_current_return_val = FWK_E_STATE;

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
    assert(FWK_LIST_GET(fwk_list_head(&fake_element_dlist_table[3]),
        struct fwk_event, slist_node) == (void *)subscription1);
    assert(fwk_id_is_equal(subscription1->source_id, FWK_ID_ELEMENT(0x2, 0x9)));
    assert(fwk_id_is_equal(subscription1->target_id, FWK_ID_MODULE(0x4)));

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
    assert(FWK_LIST_GET(fwk_list_head(&fake_module_dlist_table[1]),
        struct fwk_event, slist_node) == (void *)subscription2);
    assert(fwk_id_is_equal(subscription2->source_id, FWK_ID_MODULE(0x2)));
    assert(fwk_id_is_equal(subscription2->target_id,
                           FWK_ID_ELEMENT(0x4, 0x15)));

    /* Try to subscribe to the same module notification */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x1),
                                        FWK_ID_MODULE(0x2),
                                        FWK_ID_ELEMENT(0x4, 0x15));
    assert(result == FWK_E_STATE);
}

static void test_fwk_notification_unsubscribe(void)
{
    int result;
    struct __fwk_notification_subscription *subscription1, *subscription2,
        *subscription3;

    /* Notifications not initialized */
    result = fwk_notification_unsubscribe(FWK_ID_NOTIFICATION(0x2, 0x3),
                                          FWK_ID_ELEMENT(0x2, 0x9),
                                          FWK_ID_MODULE(0x4));
    assert(result == FWK_E_INIT);

    /* Notifications initialization */
    result = __fwk_notification_init(3);
    assert(result == FWK_SUCCESS);
    subscription1 = fwk_mm_calloc_val;
    subscription2 = subscription1 + 1;
    subscription3 = subscription2 + 1;

    /* Call from an ISR */
    interrupt_get_current_return_val = FWK_SUCCESS;
    result = fwk_notification_unsubscribe(FWK_ID_NOTIFICATION(0x2, 0x3),
                                          FWK_ID_ELEMENT(0x2, 0x9),
                                          FWK_ID_MODULE(0x4));
    assert(result == FWK_E_HANDLER);
    interrupt_get_current_return_val = FWK_E_STATE;

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
    assert(FWK_LIST_GET(fwk_list_head(&fake_element_dlist_table[1]),
        struct fwk_event, slist_node) == (void *)subscription1);

    /* Subscribe to the same element notification for another target */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x1),
                                        FWK_ID_ELEMENT(0x2, 0x9),
                                        FWK_ID_ELEMENT(0x6, 0x1));
    assert(result == FWK_SUCCESS);
    assert(FWK_LIST_GET(fwk_list_head(&fake_element_dlist_table[1]),
        struct fwk_event, slist_node) == (void *)subscription1);
    assert(subscription1->dlist_node.next == &(subscription2->dlist_node));

    /* Subscribe to a module notification */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x1),
                                        FWK_ID_MODULE(0x2),
                                        FWK_ID_ELEMENT(0x4, 0x15));
    assert(result == FWK_SUCCESS);
    assert(FWK_LIST_GET(fwk_list_head(&fake_module_dlist_table[1]),
        struct fwk_event, slist_node) == (void *)subscription3);

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
    assert(FWK_LIST_GET(fwk_list_head(&fake_element_dlist_table[1]),
        struct fwk_event, slist_node) == (void *)subscription2);

    /* Subscribe to another element notification, the subscription freed in the
       previous step is re-used */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x3),
                                        FWK_ID_MODULE(0x2),
                                        FWK_ID_ELEMENT(0xC, 0xC));
    assert(result == FWK_SUCCESS);
    assert(FWK_LIST_GET(fwk_list_head(&fake_module_dlist_table[3]),
        struct fwk_event, slist_node) == (void *)subscription1);
}

static void test_fwk_notification_notify(void)
{
    int result;
    struct fwk_event notification_event, current_event;
    unsigned int count;
    struct __fwk_notification_subscription *subscription1, *subscription2,
        *subscription3;

    /* Notifications not initialized */
    notification_event.source_id = FWK_ID_ELEMENT(0x2, 0x9);
    notification_event.id = FWK_ID_NOTIFICATION(0x2, 0x1);
    result = fwk_notification_notify(&notification_event, &count);
    assert(result == FWK_E_INIT);

    /* Notifications initialization */
    result = __fwk_notification_init(3);
    assert(result == FWK_SUCCESS);
    subscription1 = fwk_mm_calloc_val;
    subscription2 = subscription1 + 1;
    subscription3 = subscription2 + 1;

    /* Call from an ISR, invalid source identifier */
    interrupt_get_current_return_val = FWK_SUCCESS;
    is_valid_entity_id_return_val = false;
    result = fwk_notification_notify(&notification_event, &count);
    assert(result == FWK_E_PARAM);
    interrupt_get_current_return_val = FWK_E_STATE;
    is_valid_entity_id_return_val = true;

    /* Call from a thread, current event, incompatible notification and
       source identifier. */
    current_event.target_id = FWK_ID_ELEMENT(0x2, 0x9);
    notification_event.source_id = FWK_ID_ELEMENT(0x3, 0x9);
    notification_event.id = FWK_ID_NOTIFICATION(0x2, 0x1);
    get_current_event_return_val = &current_event;
    result = fwk_notification_notify(&notification_event, &count);
    assert(result == FWK_E_PARAM);
    get_current_event_return_val = NULL;

    /* Call from a thread, no current event, invalid notification identifier. */
    is_valid_notification_id_return_val = false;
    result = fwk_notification_notify(&notification_event, &count);
    assert(result == FWK_E_PARAM);
    is_valid_notification_id_return_val = true;

    /* Call from a thread, no current event, incompatible notification and
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
    assert(FWK_LIST_GET(fwk_list_head(&fake_element_dlist_table[1]),
        struct fwk_event, slist_node) == (void *)subscription1);

    /* Subscribe to the same element notification for another target */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x1),
                                        FWK_ID_ELEMENT(0x2, 0x9),
                                        FWK_ID_ELEMENT(0x6, 0x1));
    assert(result == FWK_SUCCESS);
    assert(FWK_LIST_GET(fwk_list_head(&fake_element_dlist_table[1]),
        struct fwk_event, slist_node) == (void *)subscription1);
    assert(subscription1->dlist_node.next == &(subscription2->dlist_node));

    /* Subscribe to a module notification */
    result = fwk_notification_subscribe(FWK_ID_NOTIFICATION(0x2, 0x1),
                                        FWK_ID_MODULE(0x2),
                                        FWK_ID_ELEMENT(0x4, 0x15));
    assert(result == FWK_SUCCESS);
    assert(FWK_LIST_GET(fwk_list_head(&fake_module_dlist_table[1]),
        struct fwk_event, slist_node) == (void *)subscription3);

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
    FWK_TEST_CASE(test___fwk_notification_init),
    FWK_TEST_CASE(test_fwk_notification_subscribe),
    FWK_TEST_CASE(test_fwk_notification_unsubscribe),
    FWK_TEST_CASE(test_fwk_notification_notify)
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_thread",
    .test_case_setup = test_case_setup,
    .test_case_teardown = test_case_teardown,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
