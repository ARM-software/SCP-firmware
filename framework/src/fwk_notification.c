/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Notification facilities.
 */

#include <internal/fwk_module.h>
#include <internal/fwk_notification.h>
#include <internal/fwk_thread.h>

#include <fwk_assert.h>
#include <fwk_dlist.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_list.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>

struct notification_ctx {
    /*
     * Flag indicating whether the notification framework component is
     * initialized.
     */
    bool initialized;

    /*
     * Queue of notification subscription structures that are free.
     */
    struct fwk_dlist free_subscription_dlist;
};

static struct notification_ctx ctx;

static const char err_msg_func[] = "[NOT] Error %d in %s";

/*
 * Static functions
 */

/*
 * Get list of subscriptions for a given notification emitted by a given source
 *
 * \note The function assumes the validity of all its input parameters.
 *
 * \param notification_id Identifier of the notification.
 * \param source_id Identifier of the emitter of the notification.
 *
 * \return A pointer to the doubly-linked list of subscriptions.
 */
static struct fwk_dlist *get_subscription_dlist(
    fwk_id_t notification_id, fwk_id_t source_id)
{
    struct fwk_dlist *subscription_dlist_table;

    if (fwk_id_is_type(source_id, FWK_ID_TYPE_MODULE)) {
        subscription_dlist_table =
            __fwk_module_get_ctx(source_id)->subscription_dlist_table;
    } else {
        subscription_dlist_table =
            __fwk_module_get_element_ctx(source_id)->subscription_dlist_table;
    }

    return &subscription_dlist_table[
               fwk_id_get_notification_idx(notification_id)];
}

/*
 * Search for a subscription with a given source and target identifier in a list
 * of subscriptions.
 *
 * \note The function assumes the validity of all its input parameters.
 *
 * \param subscription_dlist Pointer to the doubly-linked list of subscriptions
 *      to search.
 * \param source_id Identifier of the emitter of the notification.
 * \param target_id Identifier of the target of the notification.
 *
 * \return A pointer to the subscription that has been found if any, NULL
 *      otherwise.
 */
static struct __fwk_notification_subscription *search_subscription(
    struct fwk_dlist *subscription_dlist,
    fwk_id_t source_id, fwk_id_t target_id)
{
    struct fwk_dlist_node *node;
    struct __fwk_notification_subscription *subscription;

    for (node = fwk_list_head(subscription_dlist); node != NULL;
         node = fwk_list_next(subscription_dlist, node)) {
        subscription = FWK_LIST_GET(node,
            struct __fwk_notification_subscription, dlist_node);

        if (fwk_id_is_equal(subscription->source_id, source_id) &&
            fwk_id_is_equal(subscription->target_id, target_id))
            return subscription;
    }

    return NULL;
}

/*
 * Send all the notifications associated with a notification event.
 *
 * \note The function assumes the validity of all its input parameters. The
 *      function is a sub-routine of 'fwk_notification_notify'.
 *
 * \param notification_event Pointer to the notification event.
 * \param[out] count The number of notifications being sent.
 */
static void send_notifications(struct fwk_event *notification_event,
                               unsigned int *count)
{
    int status;
    struct fwk_dlist *subscription_dlist;
    struct fwk_dlist_node *node;
    struct __fwk_notification_subscription *subscription;

    subscription_dlist = get_subscription_dlist(notification_event->id,
                                                notification_event->source_id);
    notification_event->is_response = false;
    notification_event->is_notification = true;

    for (node = fwk_list_head(subscription_dlist); node != NULL;
         node = fwk_list_next(subscription_dlist, node)) {
        subscription = FWK_LIST_GET(node,
            struct __fwk_notification_subscription, dlist_node);

        if (!fwk_id_is_equal(subscription->source_id,
                             notification_event->source_id))
            continue;

        notification_event->target_id = subscription->target_id;

        status = __fwk_thread_put_notification(notification_event);
        if (status == FWK_SUCCESS)
            (*count)++;
    }
}

/*
 * Private interface functions
 */

int __fwk_notification_init(size_t notification_count)
{
    struct __fwk_notification_subscription *subscription_table,
        *subscription_table_upper_limit, *subscription;

    subscription_table = fwk_mm_calloc(
        notification_count, sizeof(struct __fwk_notification_subscription));

    /* All the subscription structures are free to be used. */
    fwk_list_init(&ctx.free_subscription_dlist);
    for (subscription = subscription_table,
         subscription_table_upper_limit = subscription + notification_count;
         subscription < subscription_table_upper_limit;
         subscription++)
        fwk_list_push_tail(&ctx.free_subscription_dlist,
                           &subscription->dlist_node);

    ctx.initialized = true;

    return FWK_SUCCESS;
}

void __fwk_notification_reset(void)
{
    ctx = (struct notification_ctx){ 0 };
}

/*
 * Public interface functions
 */

int fwk_notification_subscribe(fwk_id_t notification_id, fwk_id_t source_id,
                               fwk_id_t target_id)
{
    int status;
    unsigned int interrupt;
    struct fwk_dlist *subscription_dlist;
    struct __fwk_notification_subscription *subscription;

    if (!ctx.initialized) {
        status = FWK_E_INIT;
        goto error;
    }

    if (fwk_interrupt_get_current(&interrupt) == FWK_SUCCESS) {
        status = FWK_E_HANDLER;
        goto error;
    }

    if (!fwk_module_is_valid_notification_id(notification_id) ||
        !fwk_module_is_valid_entity_id(source_id) ||
        !fwk_module_is_valid_entity_id(target_id) ||
        !fwk_id_is_equal(fwk_id_build_module_id(notification_id),
                         fwk_id_build_module_id(source_id))) {
        status = FWK_E_PARAM;
        goto error;
    }

    subscription_dlist = get_subscription_dlist(notification_id, source_id);
    if (search_subscription(subscription_dlist, source_id, target_id) != NULL) {
        status = FWK_E_STATE;
        goto error;
    }

    subscription = FWK_LIST_GET(
        fwk_list_pop_head(&ctx.free_subscription_dlist),
        struct __fwk_notification_subscription, dlist_node);

    if (subscription == NULL) {
        status = FWK_E_NOMEM;
        assert(false);
        goto error;
    }

    subscription->source_id = source_id;
    subscription->target_id = target_id;

    fwk_interrupt_global_disable();
    fwk_list_push_tail(subscription_dlist, &subscription->dlist_node);
    fwk_interrupt_global_enable();

    return FWK_SUCCESS;

error:
    FWK_LOG_CRIT(err_msg_func, status, __func__);
    return status;
}

int fwk_notification_unsubscribe(fwk_id_t notification_id, fwk_id_t source_id,
                                 fwk_id_t target_id)
{
    int status;
    unsigned int interrupt;
    struct fwk_dlist *subscription_dlist;
    struct __fwk_notification_subscription *subscription;

    if (!ctx.initialized) {
        status = FWK_E_INIT;
        goto error;
    }

    if (fwk_interrupt_get_current(&interrupt) == FWK_SUCCESS) {
        status = FWK_E_HANDLER;
        goto error;
    }

    if (!(fwk_module_is_valid_notification_id(notification_id)) ||
        !(fwk_module_is_valid_entity_id(source_id)) ||
        !(fwk_module_is_valid_entity_id(target_id)) ||
        !fwk_id_is_equal(fwk_id_build_module_id(notification_id),
                         fwk_id_build_module_id(source_id))) {
        status = FWK_E_PARAM;
        goto error;
    }

    subscription_dlist = get_subscription_dlist(notification_id, source_id);
    subscription = search_subscription(subscription_dlist,
                                       source_id, target_id);
    if (subscription == NULL) {
        status = FWK_E_STATE;
        goto error;
    }

    fwk_interrupt_global_disable();
    fwk_list_remove(subscription_dlist, &subscription->dlist_node);
    fwk_interrupt_global_enable();
    fwk_list_push_tail(&ctx.free_subscription_dlist, &subscription->dlist_node);

    return FWK_SUCCESS;

error:
    FWK_LOG_CRIT(err_msg_func, status, __func__);
    return status;
}

int fwk_notification_notify(struct fwk_event *notification_event,
                            unsigned int *count)
{
    int status;
    unsigned int interrupt;
    const struct fwk_event *current_event;

    if (!ctx.initialized) {
        status = FWK_E_INIT;
        goto error;
    }

    if ((notification_event == NULL) || (count == NULL))
        return FWK_E_PARAM;

    if (fwk_interrupt_get_current(&interrupt) == FWK_SUCCESS) {
        if (!fwk_module_is_valid_entity_id(notification_event->source_id)) {
            status = FWK_E_PARAM;
            goto error;
        }
    } else {
        current_event = __fwk_thread_get_current_event();

        if ((current_event != NULL) &&
            (!fwk_module_is_valid_entity_id(notification_event->source_id))) {
            /*
             * The source_id provided is not valid, use the identifier of the
             * target for the current event.
             */
            notification_event->source_id = current_event->target_id;
        }
    }

    if (!fwk_module_is_valid_notification_id(notification_event->id) ||
        (fwk_id_get_module_idx(notification_event->id) !=
         fwk_id_get_module_idx(notification_event->source_id))) {
        status = FWK_E_PARAM;
        goto error;
    }

    *count = 0;
    send_notifications(notification_event, count);

    return FWK_SUCCESS;

error:
    FWK_LOG_CRIT(err_msg_func, status, __func__);
    return status;
}
