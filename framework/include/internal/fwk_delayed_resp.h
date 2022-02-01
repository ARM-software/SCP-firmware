/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_INTERNAL_DELAYED_RESP_H
#define FWK_INTERNAL_DELAYED_RESP_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \internal
 *
 * \brief Get the list of delayed responses for a given module or element.
 *
 * \note The function assumes the validity of all its input parameters.
 *
 * \param id Identifier of the module or element.
 *
 * \return A pointer to the list of delayed responses.
 */
struct fwk_slist *__fwk_get_delayed_response_list(fwk_id_t id);

/*!
 * \internal
 *
 * \brief Search delayed response.
 *
 * \note The function assumes the validity of all its input parameters.
 *
 * \param id Identifier of the module or element that delayed the response.
 * \param cookie Cookie of the event which the response has been delayed
 *      for. This cookie identifies the response among the several responses
 *      that the entity 'id' may have delayed.
 *
 * \return A pointer to the delayed response event, \c NULL if not found.
 */
struct fwk_event *__fwk_search_delayed_response(fwk_id_t id, uint32_t cookie);

#endif /* FWK_INTERNAL_DELAYED_RESP_H */
