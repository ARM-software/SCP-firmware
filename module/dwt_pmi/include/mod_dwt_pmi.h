/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Debug, Watchdog and Trace driver interface.
 */

#ifndef MOD_DWT_PMI_H
#define MOD_DWT_PMI_H

#include <mod_pmi.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/*!
 * \ingroup GroupModules
 * \defgroup GroupDwtPmi Performance and Instrumentation
 *      module
 * \{
 */

/*!
 * \defgroup GroupDwtPmiTypes Types
 * \{
 */

/*!
 * \brief DWT PMI (Debug Watch and Trace) registers.
 */
struct FWK_PACKED mod_dwt_pmi_config {
    /*! Debug Exception and Monitor Control Register address from SCS */
    FWK_RW uint32_t *debug_sys_demcr_addr;

    /*! Debug Watch and Trace CTRL register */
    FWK_RW uint32_t *dwt_ctrl_addr;

    /*! Debug Watch and Cycle Count register */
    FWK_RW uint32_t *dwt_cyccnt;

    /*! Generic hardware timer */
    struct cntbase_reg *hw_timer;
};

/*!
 * \}
 */

/*!
 * \defgroup GroupPmiIds Identifiers
 * \{
 */

/*!
 * \brief API indices.
 */
enum mod_dwt_pmi_api_idx {
    MOD_DWT_PMI_API_IDX_DRIVER, /*! API index */
    MOD_DWT_PMI_API_IDX_COUNT /*! Number of defined APIs */
};

/*! Module API identifier */
static const fwk_id_t mod_dwt_pmi_api_id_driver =
    FWK_ID_API_INIT(FWK_MODULE_IDX_DWT_PMI, MOD_DWT_PMI_API_IDX_DRIVER);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_DWT_PMI_H */
