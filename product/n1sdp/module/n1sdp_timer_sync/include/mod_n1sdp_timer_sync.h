/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      N1SDP Timer Synchronization Device Driver.
 */

#ifndef MOD_N1SDP_TIMER_SYNC_H
#define MOD_N1SDP_TIMER_SYNC_H

#include <fwk_id.h>
#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \addtogroup GroupN1SDPModule N1SDP Product Modules
 * @{
 */

/*!
 * \defgroup GroupN1SDPTimerSync N1SDP Timer Synchronization Driver
 * @{
 */

/*!
 * \brief Module API indices
 */
enum mod_n1sdp_timer_sync_api_idx {
    /*! Timer synchronization API */
    N1SDP_TIMER_SYNC_API_IDX_TSYNC,

    /*! Number of APIs */
    N1SDP_TIMER_SYNC_API_COUNT,
};

/*!
 * \brief N1SDP Timer Synchronization API
 */
struct n1sdp_timer_sync_api {
   /*!
    * \brief API to trigger synchronization in master.
    *
    * \param id Identifier of the timer sync module.
    *
    * \retval FWK_SUCCESS If operation succeeds.
    * \return One of the possible error return codes.
    */
   int (*master_sync)(fwk_id_t id);
   /*!
    * \brief API to trigger synchronization in slave.
    *
    * \param id Identifier of the timer sync module.
    *
    * \retval FWK_SUCCESS If operation succeeds.
    * \return One of the possible error return codes.
    */
   int (*slave_sync)(fwk_id_t id);
};

/*!
 * \brief Timer Synchronization Device Configuration
 */
struct mod_n1sdp_tsync_config {
    /*! IRQ number of the timer synchronization module */
    unsigned int irq;

    /*! Base address of the timer synchronization module */
    uintptr_t reg;

    /*! CCIX network delay */
    uint32_t ccix_delay;

    /*! GCNT sync timeout */
    uint32_t sync_timeout;

    /*! GCNT sync interval */
    uint32_t sync_interval;

    /*! GCNT offset threshold */
    uint32_t off_threshold;

    /*! Target counter base address */
    uint32_t target_cnt_base;

    /*! Offset to access target counter locally */
    uint32_t local_offset;

    /*! Offset to access target counter remotely */
    uint64_t remote_offset;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_N1SDP_TIMER_SYNC_H */
