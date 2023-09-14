/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP sub-system support.
 */

#ifndef SCP_PLATFORM_H
#define SCP_PLATFORM_H

#include <mod_scp_platform.h>

#define MOD_NAME "[SCP_PLATFORM]"

/*
 * RSS communication interface helper functions.
 */

/*!
 * \brief Helper function to return platform system transport signal API.
 *
 * \param None.
 *
 * \return Pointer to the scp platform transport signal API.
 */
const void *get_platform_transport_signal_api(void);

/*!
 * \brief Helper function to notify RSS and wait for response.
 *
 * \details Notify RSS that SYSTOP is powered up so it can enable GPC bypass in
 * the system control block and load the LCP firmware into all the instances of
 * the LCP in the platform.
 *
 * \param None.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \return One of the standard error codes for implementation-defined errors.
 */
int notify_rss_and_wait_for_response(void);

/*!
 * \brief Helper function to bind to transport and timer module APIs.
 *
 * \param config Pointer to the module config data.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \return One of the standard error codes for implementation-defined errors.
 */
int platform_rss_bind(const struct mod_scp_platform_config *config);

/*
 * LCP interface helper functions.
 */

/*!
 * \brief Helper function to setup LCP.
 *
 * \details Enable UART access for LCP0 and release all the LCPs.
 *
 * \param None.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \return One of the standard error codes for implementation-defined errors.
 */
int platform_setup_lcp(void);

/*
 * Power management interface helper functions.
 */

/*!
 * \brief Helper function to return platform system power driver API.
 *
 * \param None.
 *
 * \return Pointer to the scp platform system power driver API.
 */
const void *get_platform_system_power_driver_api(void);

/*!
 * \brief Helper function to bind to power domain restricted API.
 *
 * \param None.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \return One of the standard error codes for implementation-defined errors.
 */
int platform_power_mgmt_bind(void);

/*!
 * \brief Power on the given AP core.
 *
 * \param core_idx AP core index.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \return One of the standard error codes for implementation-defined errors.
 */
int init_ap_core(uint8_t core_idx);

#endif /* SCP_PLATFORM_H */
