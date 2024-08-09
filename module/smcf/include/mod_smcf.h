/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SMCF Driver
 */

#ifndef MOD_SMCF_H
#define MOD_SMCF_H

#include "mod_smcf_data.h"

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupModuleSMCF SMCF Driver
 * \{
 */

/*! Maximum number of mode entries as defined by the hardware spec. */
#define SMCF_MODE_ENTRY_COUNT 4

/*!
 * \brief Configuration data of a domain driver
 */
struct mod_smcf_element_config {
    /*! Base address of the MGI registers */
    uintptr_t reg_base;

    /*! SMCF IRQ number */
    unsigned int irq;

    /*! Sample type */
    enum mod_smcf_sample_type sample_type;

    /*! Data location and header format */
    struct mod_smcf_data_config data_config;
};

/*!
 * \brief SMCF notification indices.
 */
enum mod_smcf_notification_idx {
    /*! New data received notification */
    MOD_SMCF_NOTIFY_IDX_NEW_DATA_SAMPLE_READY,

    /*! Number of defined notifications */
    MOD_SMCF_NOTIFY_IDX_COUNT
};

/*!
 * \brief Id for ::MOD_SMCF_NOTIFY_IDX_NEW_DATA_SAMPLE_READY notification.
 */
static const fwk_id_t mod_smcf_notification_id_new_data_sample_ready =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_SMCF,
        MOD_SMCF_NOTIFY_IDX_NEW_DATA_SAMPLE_READY);

/*!
 * \brief SMCF buffer for copying data and tag
 */
struct mod_smcf_buffer {
    /*! Size of the buffer to copy the data into */
    uint32_t size;
    /*! Pointer to the data buffer */
    uint32_t *ptr;
};

/*!
 * \brief Indexes of the interfaces exposed by the module.
 */
enum mod_smcf_api_idx {
    /*! Data related API's */
    MOD_SMCF_API_IDX_DATA,

    /*! Control Monitor API's */
    MOD_SMCF_API_IDX_CONTROL,

    /*!
     * Interrupt handler API.
     * If another module is registered to handle the IRQ it should use this API
     */
    MOD_SMCF_API_IDX_INTERRUPT,

    /*! Number of exposed interfaces */
    MOD_SMCF_API_IDX_COUNT,
};

/*!
 * \brief SMCF module data API
 */
struct smcf_data_api {
    /*!
     * \brief Start data sampling
     *
     * \param element_id Identifier of the element
     *
     * \retval ::FWK_SUCCESS Operation successful.
     * \retval ::FWK_E_PARAM The identifier is invalid.
     */
    int (*start_data_sampling)(fwk_id_t elmenent_id);

    /*!
     * \brief Get the last data value available
     *
     * \details The caller must allocate a buffer size depending on the number
     *      of samples and the sample width. This function will unpack the data
     *      samples into 32-bit entries and copy all samples into the buffer.
     *      If the sample width is 32-bit or less, each entry will represent one
     *      sample. If the sample width is greater than 32-bits every 2 entries
     *      will represent 1 sample. Note that the maximum width of a sample
     *      is 64-bits.
     *
     * \param monitor_id Identifier of the sub-element monitor
     * \param buffer_size Size of the data buffer to copy the data into.
     *              This should be the number of 32-bits alocated in the buffer.
     * \param buffer Pointer to the data buffer. The buffer must be allocated by
     *  the caller before calling this function
     *
     * \retval ::FWK_SUCCESS Operation successful.
     * \retval ::FWK_E_PARAM The identifier is invalid.
     * \retval ::FWK_E_STATE Sampling is not started. Enable sampling first.
     * \retval ::FWK_E_NOMEM The size given is less than what is required.
     */
    int (*get_data)(
        fwk_id_t monitor_id,
        struct mod_smcf_buffer data_buffer,
        struct mod_smcf_buffer tag_buffer);
};

/*!
 * \brief SMCF module Control API
 */
struct smcf_control_api {
    /*!
     * \brief Configure mode value
     *
     * \param monitor_id Identifier of the sub-element monitor
     * \param mode_value value to be programmed
     * \param mode_index which mode to be programed
     *
     * \retval ::FWK_SUCCESS Operation successful.
     * \retval ::FWK_E_PARAM invalid parameters.
     */
    int (*config_mode)(
        fwk_id_t monitor_id,
        uint32_t mode_value,
        uint32_t mode_index);

    /*!
     * \brief Get Group ID
     *
     * \param monitor_group_id Identifier of the monitor group element
     *
     * \retval monitor group ID
     */
    uint32_t (*get_group_id)(fwk_id_t monitor_group_id);

    /*!
     * \brief Enable MLI
     *
     * \param monitor_id Identifier of the sub-element monitor
     *
     * \retval ::FWK_SUCCESS Operation successful.
     * \retval ::FWK_E_PARAM invalid parameters.
     */
    int (*mli_enable)(fwk_id_t monitor_id);

    /*!
     * \brief Disable MLI
     *
     * \param monitor_id Identifier of the sub-element monitor
     *
     * \retval ::FWK_SUCCESS Operation successful.
     * \retval ::FWK_E_PARAM invalid parameters.
     */
    int (*mli_disable)(fwk_id_t monitor_id);
};

/*!
 * \brief SMCF module Interrupt API
 *
 * \details There is a situation where another module needs to handle the SMCF
 *      interrupt. For example, the hardware multiplex the IRQ line with several
 *      peripherals, including SMCF. In this scenario, a module will receive the
 *      IRQ and read the hardware registers to identify the interrupt source.
 *      If SMCF is the IRQ source, the module can use this API to handle the
 *      interrupt.
 */
struct smcf_monitor_group_interrupt_api {
    /*!
     * \brief Handle SMCF interrupt
     *
     * \param monitor_group_id Identifier of the monitor_group
     *
     * \retval ::FWK_SUCCESS Operation successful.
     * \retval ::FWK_E_PARAM The identifier is invalid.
     */
    int (*handle_interrupt)(fwk_id_t monitor_group_id);
};

/*!
 * \}
 */

/*!
 * \}
 */
#endif /* MOD_SMCF_H */
