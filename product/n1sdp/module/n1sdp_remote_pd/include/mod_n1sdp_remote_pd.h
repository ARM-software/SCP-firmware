/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      N1SDP Remote Power Domain (PD) management driver.
 */

#ifndef MOD_N1SDP_REMOTE_PD_H
#define MOD_N1SDP_REMOTE_PD_H

#include <mod_power_domain.h>

/*!
 * \addtogroup GroupN1SDPModule N1SDP Product Modules
 * \{
 */

/*!
 * \defgroup GroupModuleN1SDPRemotePD N1SDP Remote PD Management Driver
 * \{
 */

/*!
 * \brief Remote PD element configuration
 */
struct mod_n1sdp_remote_pd_config {
    /*! Power domain type */
    enum mod_pd_type pd_type;
};

/*!
 * \brief Module API indices
 */
enum mod_n1sdp_remote_pd_api {
    /*! Index of the remote PD API */
    N1SDP_REMOTE_PD_API_IDX,

    /*! Number of APIs */
    N1SDP_REMOTE_PD_API_COUNT
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_N1SDP_REMOTE_PD_H */
