/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_INTERNAL_NOTIFICATION_H
#define FWK_INTERNAL_NOTIFICATION_H

#include <fwk_list.h>
#include <fwk_notification.h>

#include <stdbool.h>
#include <stddef.h>

struct __fwk_notification_subscription {
    struct fwk_dlist_node dlist_node;

    /* Identifier of the notification source entity. */
    fwk_id_t source_id;

    /* Identifier of the notification target entity. */
    fwk_id_t target_id;
};

/*
 * \brief Initialize the notification framework component.
 *
 * \param notification_count The maximum number of notification subscriptions at
 *      any time.
 *
 * \retval FWK_SUCCESS The notification framework component was initialized.
 * \retval FWK_E_PARAM The maximum number of notification subscriptions is equal
 *      to zero.
 * \retval FWK_E_NOMEM Insufficient memory available to allocate the
 *      notification subscription.
 */
int __fwk_notification_init(size_t notification_count);

/*
 * \brief Reset the notification framework component.
 *
 * \note Only for testing.
 */
void __fwk_notification_reset(void);

#endif /* FWK_INTERNAL_NOTIFICATION_H */
