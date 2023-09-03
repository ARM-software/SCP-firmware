/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_AMU_MMAP_H
#define MOD_AMU_MMAP_H

#include <interface_amu.h>

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \ingroup GroupModules
 *  \defgroup GroupAmuMmap AMU Memory Mapped  (AMU-MMAP)
 * \{
 */

/*!
 * \defgroup GroupAmuMmapApis APIs
 * \{
 */

/*!
 * \brief API indices
 */
enum mod_amu_mmap_api_idx {
    MOD_AMU_MMAP_API_IDX_AMU,
    MOD_AMU_MMAP_API_IDX_COUNT,
};

/*! Module API identifier */
static const fwk_id_t mod_amu_mmap_api_id_amu =
    FWK_ID_API_INIT(FWK_MODULE_IDX_AMU_MMAP, MOD_AMU_MMAP_API_IDX_AMU);

/*!
 * \}
 */

/*!
 * \defgroup GroupAmuMmapConfig Configuration
 * \{
 */

/*!
 * \brief amu_mmap element config (core config).
 */
struct mod_core_element_config {
    /*! Base address of a Core's AMU counters */
    uint64_t *counters_base_addr;
    /*! Table of AMU counters offsets (in bytes) of a core */
    uint32_t *counters_offsets;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_AMU_MMAP_H */
