/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     MHU module device indexes.
 */

#ifndef SCP_TC_MHU_H
#define SCP_TC_MHU_H

enum scp_tc_mhu_device_idx {
    SCP_TC_MHU_DEVICE_IDX_SCP_AP_S_CLUS0,
    SCP_TC_MHU_DEVICE_IDX_SCP_AP_NS_HP_CLUS0,
    SCP_TC_MHU_DEVICE_IDX_SCP_AP_NS_LP_CLUS0,
    SCP_TC_MHU_DEVICE_IDX_SCP_RSS_S_CLUS0,
    SCP_TC_MHU_DEVICE_IDX_COUNT
};

#endif /* SCP_TC_MHU_H */
