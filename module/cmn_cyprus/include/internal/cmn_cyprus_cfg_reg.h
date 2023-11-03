/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for accessing configuration register.
 */

#ifndef CMN_CYPRUS_CFG_REG_INTERNAL_H
#define CMN_CYPRUS_CFG_REG_INTERNAL_H

#include <internal/cmn_cyprus_reg.h>

/*
 * Retrieve the CMN Cyprus revision.
 *
 * \param cfgm Pointer to the global configuration completer register.
 *      \pre The pointer must be valid.
 *
 * \return CMN Cyprus revision.
 */
unsigned int cfgm_get_cmn_revision(struct cmn_cyprus_cfgm_reg *cfgm);

/*
 * Get the child node configuration register pointer.
 *
 * \param cfgm Pointer to the global configuration completer register.
 *      \pre The pointer must be valid.
 * \param child_index Child node index.
 *      \pre Child node index must be valid.
 *
 * \return Pointer to the child configuration register.
 */
struct cmn_cyprus_node_cfg_reg *cfgm_get_child_node(
    struct cmn_cyprus_cfgm_reg *cfgm,
    unsigned int child_index);

#endif /* CMN_CYPRUS_CFG_REG_INTERNAL_H */
