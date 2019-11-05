/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Reset domain HAL.
 */

#ifndef MOD_RESET_DOMAIN_H
#define MOD_RESET_DOMAIN_H

#include <fwk_id.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupResetDomain Reset Domains HAL
 *
 * \details Support for setting the state of reset domains.
 *
 * @{
 */

/*!
 * \brief APIs that the module makes available to entities requesting binding.
 */
enum mod_reset_domain_api_type {
    /*! Reset Domain HAL */
    MOD_RESET_DOMAIN_API_TYPE_HAL,
    MOD_RESET_DOMAIN_API_COUNT,
};


#ifdef BUILD_HAS_NOTIFICATION
/*!
 * \brief Reset domain module configuration data.
 *
 *  TODO: Needed for notification, not yet implemented in reset_domain module
 */
struct mod_reset_domain_config {
    /*!
     * \brief Identifier of a notification to subscribe reset domain devices to
     *     in order to receive notifications of reset domains that have already
     *     occurred.
     *
     * \note May be \ref FWK_ID_NONE to disable this functionality for all
     *     elements.
     */
    const fwk_id_t notification_id;
};
#endif

/*!
 * \brief Reset domain element configuration data.
 */
struct mod_reset_domain_dev_config {
    /*! Driver identifier */
    const fwk_id_t driver_id;
    /*! Driver API identifier */
    const fwk_id_t api_id;
};

/*!
 * \brief Reset domain driver interface.
 *
 * \details The interface the reset domain module relies on to perform
 *      actions on a reset domain.
 */

struct mod_reset_domain_drv_api {
    /*! Name of the driver */
    const char *name;

    /*!
     * \brief Target reset domain \p dev_id performs an autonomous reset.
     *
     * \param dev_id Driver identifier of the reset domain.
     *
     * \retval FWK_SUCCESS or one of FWK_E_* error codes.
     */
    int (*auto_domain)(fwk_id_t dev_id, unsigned int state);

    /*!
     * \brief Target reset domain identified by \p dev_id enters reset state.
     *
     * \param dev_id Driver identifier of the reset domain.
     *
     * \retval FWK_SUCCESS or one of FWK_E_* error codes.
     */
    int (*assert_domain)(fwk_id_t dev_id);

    /*!
     * \brief Target reset domain identified by \p dev_id exits reset state.
     *
     * \param dev_id Driver identifier of the reset domain.
     *
     * \retval FWK_SUCCESS or one of FWK_E_* error codes.
     */
    int (*deassert_domain)(fwk_id_t dev_id);
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_RESET_DOMAIN_H */
