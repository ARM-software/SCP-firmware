/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     AMU interface
 */

#ifndef INTERFACE_AMU_H
#define INTERFACE_AMU_H

#include <fwk_id.h>

#include <stddef.h>

/*!
 * \addtogroup GroupInterfaces Interfaces
 * \{
 */

/*!
 * \defgroup GroupAmu Activity Monitor Unit counters interface
 *
 * \brief Interface definition for AMU drivers.
 *
 * \details This interface should be implemented by drivers to access
 *          AMU counters.
 * \{
 */

/*!
 * \defgroup GroupAmuApis APIs
 * \{
 */

/*!
 * \brief AMU API to retrieve the AMU counters.
 */
struct amu_api {
    /*!
     * \brief Get the AMU counters value starting from the given counter ID
     *
     * \param start_counter_id ID of the counter to start from.
     * \param[out] counter_buff Pointer to a buffer to be filled with the
     *                          counters values.
     * \param num_counter The number of the counters requested.
     *
     * \note \b counter_buff must have space for \b num_counter elements.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     * \retval ::FWK_E_RANGE Number of counters requested is out of range.
     * \retval ::FWK_SUCCESS The request was successfully completed.
     * \return One of the standard framework status codes.
     */
    int (*get_counters)(
        fwk_id_t start_counter_id,
        uint64_t *counter_buff,
        size_t num_counter);
};

/*!
 * \}
 */

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* INTERFACE_AMU_H */
