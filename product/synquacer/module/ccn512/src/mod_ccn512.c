/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "synquacer_ddr.h"

#include <internal/ccn512.h>

#include <mod_ccn512.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

#include <stddef.h>
#include <stdint.h>

#define HNF_COUNT 8
#define SNF_ID_DMC1 0x8ULL
#define SNF_ID_DMC3 0x1AULL

#define SNF_MP_ID_DMC0 0x8ULL
#define SNF_MP_ID_DMC1 0x1AULL

static void ccn512_qos_init(ccn512_reg_t *ccn512)
{
    /*
     * Setting QOS priority for each CPU cluster to 0xE and setting the
     * enable bit so the new setting takes effect.  This function must be
     * called when there are no ongoing transactions on the ports being
     * configured, so it must be called after powering on SYSTOP and before
     * the applications cores/clusters are released from reset.
     *
     * See "Device 0 Port QoS Control register" in the Corelink CCN512 Cache
     * Coherent Network technical reference manual.
     */
    ccn512->XP_ID_0.DEV0_QOS_CONTROL = 0xE0004; /* Cluster 0   */
    ccn512->XP_ID_1.DEV0_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_2.DEV0_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_3.DEV0_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_4.DEV0_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_5.DEV0_QOS_CONTROL = 0xE0004; /* PCIE1       */
    ccn512->XP_ID_6.DEV0_QOS_CONTROL = 0xE0004; /* Cluster 6   */
    ccn512->XP_ID_7.DEV0_QOS_CONTROL = 0xE0004; /* Cluster 8   */
    ccn512->XP_ID_8.DEV0_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_9.DEV0_QOS_CONTROL = 0xE0004; /* Cluster 1   */
    ccn512->XP_ID_10.DEV0_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_11.DEV0_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_12.DEV0_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_13.DEV0_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_14.DEV0_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_15.DEV0_QOS_CONTROL = 0xE0004; /* Cluster 7   */
    ccn512->XP_ID_16.DEV0_QOS_CONTROL = 0xE0004; /* Cluster 9   */
    ccn512->XP_ID_17.DEV0_QOS_CONTROL = 0;

    ccn512->XP_ID_0.DEV1_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_1.DEV1_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_2.DEV1_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_3.DEV1_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_4.DEV1_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_5.DEV1_QOS_CONTROL = 0xE0004; /* Cluster 2   */
    ccn512->XP_ID_6.DEV1_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_7.DEV1_QOS_CONTROL = 0xE0004; /* Cluster 10  */
    ccn512->XP_ID_8.DEV1_QOS_CONTROL = 0xE0004; /* Cluster 5   */
    ccn512->XP_ID_9.DEV1_QOS_CONTROL = 0xE0004; /* PCIE0       */
    ccn512->XP_ID_10.DEV1_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_11.DEV1_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_12.DEV1_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_13.DEV1_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_14.DEV1_QOS_CONTROL = 0xE0004; /* Cluster 3   */
    ccn512->XP_ID_15.DEV1_QOS_CONTROL = 0; /* not Cluster */
    ccn512->XP_ID_16.DEV1_QOS_CONTROL = 0xE0004; /* Cluster 11  */
    ccn512->XP_ID_17.DEV1_QOS_CONTROL = 0xE0004; /* Cluster 4   */
}

static void ccn512_dmc_init(ccn512_reg_t *ccn512)
{
    unsigned int i;
    ccn5xx_hnf_reg_t *hnf = &ccn512->HNF_ID_2;
    uint64_t ddr_ch0_id, ddr_ch1_id;

    uint8_t ddr_memory_used_ch;
    ddr_memory_used_ch = fw_get_used_memory_ch();

    ddr_ch0_id = SNF_MP_ID_DMC0;
    ddr_ch1_id = SNF_MP_ID_DMC1;

    /*
     * On SynQuacer, there are only 2 DMCs (IDs 1 and 3).
     * All traffic from the first half of the HN-Fs must go to DMC1 and the
     * second half to DMC3.
     * If this setup is not done, the traffic for the DMCs that do not
     * exist will lock-up the system.
     */
    for (i = 0; i < HNF_COUNT; i++) {
        if (ddr_memory_used_ch == DDR_USE_CH0) {
            hnf[i].HNF_SAM_CONTROL = ddr_ch0_id;
        } else if (ddr_memory_used_ch == DDR_USE_CH1) {
            hnf[i].HNF_SAM_CONTROL = ddr_ch1_id;
        } else {
            hnf[i].HNF_SAM_CONTROL =
                (i < (HNF_COUNT / 2) ? ddr_ch0_id : ddr_ch1_id);
        }
    }
}

static void ccn512_secure_init(ccn512_reg_t *ccn512)
{
    /* set Non-secure access enable */
    ccn512->MN_ID_34.SECURE_ACCESS |= 0x1;
}

void fw_ccn512_init(ccn512_reg_t *ccn512)
{
    ccn512_secure_init(ccn512);
    ccn512_qos_init(ccn512);
    ccn512_dmc_init(ccn512);

    /* Wait for write operations to finish. */
    __DMB();
}

void fw_ccn512_exit(void)
{
    const struct mod_ccn512_module_config *module_config;
    ccn512_reg_t *ccn512;
    unsigned int i;

    module_config = fwk_module_get_data(fwk_module_id_ccn512);
    assert(module_config != NULL);

    ccn512 = module_config->reg_base;

    ccn5xx_hnf_reg_t *hnf = &ccn512->HNF_ID_2;

    FWK_LOG_INFO("[CCN512] CCN512 exit.");

    /* exit ALL CA53 CPU SNOOP */
    for (i = 0; i < HNF_COUNT; i++)
        hnf[i].SNOOP_DOMAIN_CTL_CLR = hnf[i].SNOOP_DOMAIN_CTL;

    /* Wait for write operations to finish. */
    __DMB();

    FWK_LOG_INFO("[CCN512] CCN512 exit end.");
}

static int ccn512_config(ccn512_reg_t *ccn512)
{
    FWK_LOG_INFO("[CCN512] Initialising ccn512 at 0x%x", (uintptr_t)ccn512);

    fw_ccn512_init(ccn512);

    FWK_LOG_INFO("[CCN512] CCN512 init done.");

    return FWK_SUCCESS;
}

static struct mod_ccn512_api module_api = {
    .ccn512_exit = fw_ccn512_exit,
};

/* Framework API */
static int mod_ccn512_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    return FWK_SUCCESS;
}

static int mod_ccn512_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    assert(data != NULL);

    return FWK_SUCCESS;
}

static int mod_ccn512_bind(fwk_id_t id, unsigned int round)
{
    /* Nothing to do in the second round of calls. */
    if (round == 1)
        return FWK_SUCCESS;

    /* Nothing to do in case of elements. */
    if (fwk_module_is_valid_element_id(id))
        return FWK_SUCCESS;

    return FWK_SUCCESS;
}

static int mod_ccn512_start(fwk_id_t id)
{
    const struct mod_ccn512_module_config *module_config;
    ccn512_reg_t *ccn512;

    module_config = fwk_module_get_data(fwk_module_id_ccn512);
    assert(module_config != NULL);

    ccn512 = module_config->reg_base;

    return ccn512_config(ccn512);
}

static int mod_ccn512_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    *api = &module_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_ccn512 = {
    .name = "ccn512",
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = mod_ccn512_init,
    .element_init = mod_ccn512_element_init,
    .bind = mod_ccn512_bind,
    .start = mod_ccn512_start,
    .process_bind_request = mod_ccn512_process_bind_request,
    .api_count = 1,
    .event_count = 0,
};
