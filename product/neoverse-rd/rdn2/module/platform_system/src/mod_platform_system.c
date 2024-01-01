/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     RD-N2 System Support.
 */

#include "clock_soc.h"
#include "platform_core.h"
#include "platform_scmi.h"
#include "platform_sds.h"
#include "scp_css_mmap.h"
#include "scp_pik.h"

#include <mod_apremap.h>
#include <mod_clock.h>
#include <mod_platform_system.h>
#include <mod_power_domain.h>
#include <mod_ppu_v1.h>
#include <mod_scmi.h>
#include <mod_sds.h>
#include <mod_system_info.h>
#include <mod_system_power.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

#include <stdbool.h>
#include <stdint.h>

/* SCMI services required to enable the messaging stack */
static unsigned int scmi_notification_table[] = {
    SCP_PLATFORM_SCMI_SERVICE_IDX_PSCI,
};

/* Module context */
struct platform_system_ctx {
    /* Pointer to the SCP PIK registers */
    struct pik_scp_reg *pik_scp_reg;

    /* Pointer to the Interrupt Service Routine API of the PPU_V1 module */
    const struct ppu_v1_isr_api *ppu_v1_isr_api;

    /* Power domain module restricted API pointer */
    struct mod_pd_restricted_api *mod_pd_restricted_api;

    /* SDS API pointer */
    const struct mod_sds_api *sds_api;

    /* CMN Address Translation API */
    const struct mod_apremap_cmn_atrans_api *apremap_cmn_atrans_api;

    /* System Information HAL API pointer */
    struct mod_system_info_get_info_api *system_info_api;

    /* Config containig data required for platform initialization */
    const struct mod_platform_system_config *config;

    /* Count of number of warm reset completion check iterations */
    unsigned int warm_reset_check_cnt;
};

struct platform_system_isr {
    unsigned int interrupt;
    void (*handler)(void);
};

static struct platform_system_ctx platform_system_ctx;

static const uint32_t feature_flags =
    (PLATFORM_SDS_FEATURE_FIRMWARE_MASK | PLATFORM_SDS_FEATURE_DMC_MASK |
     PLATFORM_SDS_FEATURE_MESSAGING_MASK);

static fwk_id_t sds_feature_availability_id = FWK_ID_ELEMENT_INIT(
    FWK_MODULE_IDX_SDS,
    PLATFORM_SDS_FEATURE_AVAILABILITY_IDX);

static fwk_id_t sds_reset_syndrome_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SDS, PLATFORM_SDS_RESET_SYNDROME_IDX);

static fwk_id_t sds_cpu_info_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SDS, PLATFORM_SDS_CPU_INFO_IDX);

static fwk_id_t sds_isolated_cpu_mpid_list_id = FWK_ID_ELEMENT_INIT(
    FWK_MODULE_IDX_SDS,
    PLATFORM_SDS_ISOLATED_CPU_MPID_LIST_IDX);

/*
 *  SCMI Messaging stack
 */

static int messaging_stack_ready(void)
{
    const struct mod_sds_structure_desc *sds_structure_desc =
        fwk_module_get_data(sds_feature_availability_id);

    /*
     * Write SDS Feature Availability to signal the completion of the messaging
     * stack
     */
    return platform_system_ctx.sds_api->struct_write(
        sds_structure_desc->id,
        0,
        (void *)(&feature_flags),
        sds_structure_desc->size);
}

/*
 * The SCP PIK address space (0x5000.0000 to 0x5FFF.FFFF) should be read with
 * 32-bit aligned addresses. SDS module performs reads at byte-aligned address.
 * So this helper function is used to first read the Reset Syndrome value from
 * the RESET_SYNDROME register and update the reset syndrome value using SDS
 * write API.
 */
static int update_sds_reset_syndrome(void)
{
    uint32_t reset_syndrome;
    const struct mod_sds_structure_desc *sds_structure_desc =
        fwk_module_get_data(sds_reset_syndrome_id);

    /* Write SDS Reset Syndrome value */
    reset_syndrome = SCP_PIK_PTR->RESET_SYNDROME;
    return platform_system_ctx.sds_api->struct_write(
        sds_structure_desc->id,
        0,
        (void *)(&reset_syndrome),
        sds_structure_desc->size);
}

/*
 * Helper function to update the primary CPU number in SDS.
 */
static int update_primary_cpu_in_sds(void)
{
    const struct mod_sds_structure_desc *sds_structure_desc =
        fwk_module_get_data(sds_cpu_info_id);
    uint8_t primary_cpu;

    primary_cpu =
        platform_calc_core_pos(platform_system_ctx.config->primary_cpu_mpid);

    return platform_system_ctx.sds_api->struct_write(
        sds_structure_desc->id, 0, (void *)(&primary_cpu), sizeof(primary_cpu));
}

/*
 * Helper function to check if a cpu is in isolated CPU MPID list.
 */
static bool is_cpu_isolated(uint64_t cpu_mpid)
{
    uint64_t isolated_cpu_count;
    uint64_t *isolated_cpu_mpid_list;

    isolated_cpu_count =
        platform_system_ctx.config->isolated_cpu_info.isolated_cpu_count;
    isolated_cpu_mpid_list =
        platform_system_ctx.config->isolated_cpu_info.isolated_cpu_mpid_list;

    while (isolated_cpu_count != 0) {
        if (isolated_cpu_mpid_list[isolated_cpu_count - 1] == cpu_mpid) {
            return true;
        }
        isolated_cpu_count--;
    }

    return false;
}

/*
 * Helper function to validate the configuration data received during init.
 */
static int validate_config_data(void)
{
    if (is_cpu_isolated(platform_system_ctx.config->primary_cpu_mpid)) {
        FWK_LOG_ERR("[PLATFORM SYSTEM] Found primary CPU in isolated CPU list");
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

/*
 * Helper function to update the isolated CPU MPID list in SDS.
 */
static int update_isolated_mpid_list_in_sds(void)
{
    const struct mod_sds_structure_desc *sds_structure_desc =
        fwk_module_get_data(sds_isolated_cpu_mpid_list_id);
    const struct mod_platform_system_config *config =
        platform_system_ctx.config;
    uint64_t isolated_cpu_count;
    int status;

    isolated_cpu_count = config->isolated_cpu_info.isolated_cpu_count;
    if (isolated_cpu_count == 0) {
        return FWK_SUCCESS;
    }

    status = platform_system_ctx.sds_api->struct_write(
        sds_structure_desc->id,
        0,
        &isolated_cpu_count,
        sizeof(isolated_cpu_count));
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = platform_system_ctx.sds_api->struct_write(
        sds_structure_desc->id,
        sizeof(isolated_cpu_count),
        config->isolated_cpu_info.isolated_cpu_mpid_list,
        (sizeof(config->isolated_cpu_info.isolated_cpu_mpid_list[0]) *
         isolated_cpu_count));

    return status;
}

/*
 *  PPU Interrupt Service Routines for cluster and core power domains
 */

static void ppu_cores_isr(unsigned int first, uint32_t status)
{
    unsigned int core_idx;

    while (status != 0) {
        core_idx = __builtin_ctz(status);
        status &= ~(1 << core_idx);

        if ((first + core_idx) >= platform_get_core_count()) {
            continue;
        }

        platform_system_ctx.ppu_v1_isr_api->ppu_interrupt_handler(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PPU_V1, first + core_idx));
    }
}

static void ppu_cores_isr_0(void)
{
    ppu_cores_isr(0, platform_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[0]);
}

static void ppu_cores_isr_1(void)
{
    ppu_cores_isr(32, platform_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[1]);
}

static void ppu_cores_isr_2(void)
{
    ppu_cores_isr(64, platform_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[2]);
}

static void ppu_cores_isr_3(void)
{
    ppu_cores_isr(96, platform_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[3]);
}

static void ppu_clusters_isr(void)
{
    uint32_t status = platform_system_ctx.pik_scp_reg->CLUS_PPU_INT_STATUS[0];
    unsigned int cluster_idx;

    while (status != 0) {
        cluster_idx = __builtin_ctz(status);

        platform_system_ctx.ppu_v1_isr_api->ppu_interrupt_handler(
            FWK_ID_ELEMENT(
                FWK_MODULE_IDX_PPU_V1,
                platform_get_core_count() + cluster_idx));

        status &= ~(1 << cluster_idx);
    }
}

/*
 *  PPU Interrupt Service Routine table
 */

static struct platform_system_isr isrs[] = {
    [0] = { .interrupt = PPU_CORES0_IRQ, .handler = ppu_cores_isr_0 },
    [1] = { .interrupt = PPU_CORES1_IRQ, .handler = ppu_cores_isr_1 },
    [2] = { .interrupt = PPU_CORES2_IRQ, .handler = ppu_cores_isr_2 },
    [3] = { .interrupt = PPU_CORES3_IRQ, .handler = ppu_cores_isr_3 },
    [4] = { .interrupt = PPU_CLUSTERS_IRQ, .handler = ppu_clusters_isr },
};

/*
 * System power's driver API
 */

static int platform_system_shutdown(enum mod_pd_system_shutdown system_shutdown)
{
    NVIC_SystemReset();

    return FWK_E_DEVICE;
}

static const struct mod_system_power_driver_api
    platform_system_system_power_driver_api = {
        .system_shutdown = platform_system_shutdown,
    };

/*
 * Functions fulfilling the framework's module interface
 */

static int platform_system_mod_init(
    fwk_id_t module_id,
    unsigned int unused,
    const void *data)
{
    int status;
    unsigned int idx;
    struct platform_system_isr *isr;
#if (PLATFORM_VARIANT == 3)
    unsigned int refclk_cur_source, refclk125_timeout = 1000;
#endif

    if (data == NULL) {
        return FWK_E_PARAM;
    }

    platform_system_ctx.config = data;

    status = validate_config_data();
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[PLATFORM SYSTEM] Configuration data is invalid");
        return status;
    }

    for (idx = 0; idx < FWK_ARRAY_SIZE(isrs); idx++) {
        isr = &isrs[idx];
        status = fwk_interrupt_set_isr(isr->interrupt, isr->handler);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    platform_system_ctx.pik_scp_reg = (struct pik_scp_reg *)SCP_PIK_SCP_BASE;

#if (PLATFORM_VARIANT == 3)
    /* Switch REFCLK clock source from 100MHz to 125MHz. */
    platform_system_ctx.pik_scp_reg->REFCLK_CTRL =
        REFCLK_CTRL_CLKSELECT_REFCLK_125;
    while (refclk125_timeout > 0) {
        refclk_cur_source = (platform_system_ctx.pik_scp_reg->REFCLK_CTRL &
                             REFCLK_CTRL_CLKSELECT_CUR_MASK) >>
            REFCLK_CTRL_CLKSELECT_CUR_SHIFT;

        if (refclk_cur_source == REFCLK_CTRL_CLKSELECT_REFCLK_125) {
            break;
        }

        refclk125_timeout--;
    }

    if (refclk125_timeout == 0) {
        FWK_LOG_ERR(
            "[PLATFORM SYSTEM] Failed to switch to 125MHz REFCLK source for "
            "system counter!");
        return FWK_E_TIMEOUT;
    }

    FWK_LOG_INFO(
        "[PLATFORM SYSTEM] REFCLK_CTRL: %lx",
        (unsigned long)platform_system_ctx.pik_scp_reg->REFCLK_CTRL);
#endif

    return FWK_SUCCESS;
}

static int platform_system_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round > 0) {
        return FWK_SUCCESS;
    }

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_DOMAIN),
        FWK_ID_API(FWK_MODULE_IDX_POWER_DOMAIN, MOD_PD_API_IDX_RESTRICTED),
        &platform_system_ctx.mod_pd_restricted_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_PPU_V1),
        FWK_ID_API(FWK_MODULE_IDX_PPU_V1, MOD_PPU_V1_API_IDX_ISR),
        &platform_system_ctx.ppu_v1_isr_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_APREMAP),
        FWK_ID_API(FWK_MODULE_IDX_APREMAP, MOD_APREMAP_API_IDX_CMN_ATRANS),
        &platform_system_ctx.apremap_cmn_atrans_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SYSTEM_INFO),
        FWK_ID_API(FWK_MODULE_IDX_SYSTEM_INFO, MOD_SYSTEM_INFO_GET_API_IDX),
        &platform_system_ctx.system_info_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return fwk_module_bind(
        fwk_module_id_sds,
        FWK_ID_API(FWK_MODULE_IDX_SDS, 0),
        &platform_system_ctx.sds_api);
}

static int platform_system_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t pd_id,
    fwk_id_t api_id,
    const void **api)
{
    *api = &platform_system_system_power_driver_api;
    return FWK_SUCCESS;
}

static int platform_system_start(fwk_id_t id)
{
    const struct mod_system_info *system_info;
    uint8_t chip_id;
    uint8_t primary_cpu;
    uint8_t primary_chip;
    int status;
    unsigned int i;

    status = platform_system_ctx.system_info_api->get_system_info(&system_info);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[PLATFORM SYSTEM] Failed to obtain system information");
        return FWK_E_PANIC;
    }
    chip_id = system_info->chip_id;

    /* Determine the chip from which the first application core boots */
    primary_cpu =
        platform_calc_core_pos(platform_system_ctx.config->primary_cpu_mpid);
    primary_chip = primary_cpu / MAX_PE_PER_CHIP;
    FWK_LOG_INFO(
        "[PLATFORM SYSTEM] CPU-%d on Chip-%d is boot CPU",
        primary_cpu % MAX_PE_PER_CHIP,
        primary_chip);

    /*
     * Subscribe to interconnect clock state change and SDS notification only
     * for the chip on which the first application core starts the boot
     * process.
     */
    if (chip_id == primary_chip) {
        /*
         * Subscribe to interconnect clock state change notification to
         * to power up the first application core.
         */
        status = fwk_notification_subscribe(
            mod_clock_notification_id_state_changed,
            FWK_ID_ELEMENT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_INTERCONNECT),
            id);
        if (status != FWK_SUCCESS) {
            return status;
        }

        /*
         * Subscribe to the SDS initialized notification so we can correctly let
         * the PSCI agent know that the SCMI stack is initialized.
         */
        status = fwk_notification_subscribe(
            mod_sds_notification_id_initialized, fwk_module_id_sds, id);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    /* Subscribe to warm reset notifications */
    status = fwk_notification_subscribe(
        mod_pd_notification_id_pre_warm_reset,
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_DOMAIN),
        id);
    if (status != FWK_SUCCESS) {
        FWK_LOG_WARN(
            "[PLATFORM SYSTEM] failed to subscribe to warm reset "
            "notification");
    }

    /*
     * This platform has the CMN configuration register in the AP address space
     * which can be accessed by enabling the CMN Address Translation.
     */
    platform_system_ctx.apremap_cmn_atrans_api->enable();

    /*
     * Subscribe to these SCMI channels in order to know when they have all
     * initialized.
     * At that point we can consider the SCMI stack to be initialized from
     * the point of view of the PSCI agent.
     */
    for (i = 0; i < FWK_ARRAY_SIZE(scmi_notification_table); i++) {
        status = fwk_notification_subscribe(
            mod_scmi_notification_id_initialized,
            fwk_id_build_element_id(
                fwk_module_id_scmi, scmi_notification_table[i]),
            id);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return status;
}

static void power_off_all_cores(void)
{
    unsigned int pd_idx;
    unsigned int core_count;
    int status;
    struct mod_pd_restricted_api *mod_pd_restricted_api =
        platform_system_ctx.mod_pd_restricted_api;

    core_count = platform_get_core_count();

    for (pd_idx = 0; pd_idx < core_count; pd_idx++) {
        FWK_LOG_INFO(
            "[PLATFORM SYSTEM] Powering down %s",
            fwk_module_get_element_name(
                FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, pd_idx)));

        status = mod_pd_restricted_api->set_state(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, pd_idx),
            false,
            MOD_PD_COMPOSITE_STATE(MOD_PD_LEVEL_0, 0, 0, 0, MOD_PD_STATE_OFF));

        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                "[PLATFORM SYSTEM] Power down of %s failed",
                fwk_module_get_element_name(
                    FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, pd_idx)));
        }
        fwk_assert(status == FWK_SUCCESS);
    }
}

static int check_power_off_all_cores(void)
{
    unsigned int core_count;
    unsigned int power_state;
    unsigned int pd_idx;
    int status = 0;
    struct mod_pd_restricted_api *mod_pd_restricted_api =
        platform_system_ctx.mod_pd_restricted_api;

    core_count = platform_get_core_count();

    /* Check if all the CPU power domain are powered down */
    for (pd_idx = 0; pd_idx < core_count; pd_idx++) {
        status = mod_pd_restricted_api->get_state(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, pd_idx), &power_state);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                "[PLATFORM_SYSTEM] failed to get state of %s",
                fwk_module_get_element_name(
                    FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, pd_idx)));
            return status;
        }

        /* Exit if any core is not powered down */
        if ((power_state &
             (MOD_PD_CS_STATE_MASK << MOD_PD_CS_LEVEL_0_STATE_SHIFT)) !=
            MOD_PD_STATE_OFF) {
            FWK_LOG_INFO(
                "[PLATFORM_SYSTEM] %s not yet powered down",
                fwk_module_get_element_name(
                    FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, pd_idx)));
            return FWK_PENDING;
        }
    }
    return status;
}

static void boot_primary_core(void)
{
    uint8_t boot_cpu;
    int status;
    struct mod_pd_restricted_api *mod_pd_restricted_api =
        platform_system_ctx.mod_pd_restricted_api;

    status = messaging_stack_ready();
    if (status != FWK_SUCCESS) {
        FWK_LOG_WARN(
            "[PLATFORM_SYSTEM] Failed to update SDS feature "
            "availability");
    }
    fwk_assert(status == FWK_SUCCESS);

    status = update_sds_reset_syndrome();
    if (status != FWK_SUCCESS) {
        FWK_LOG_WARN(
            "[PLATFORM_SYSTEM] Failed to update SDS reset "
            "syndrome");
    }
    fwk_assert(status == FWK_SUCCESS);

    FWK_LOG_INFO(
        "[PLATFORM SYSTEM] Warm reset complete. Powering up "
        "boot cpu...");

    /*
     * All cores were powered down. Now power up the primary core to
     * complete the warm reboot sequence.
     */

    boot_cpu =
        platform_calc_core_pos(platform_system_ctx.config->primary_cpu_mpid) %
        MAX_PE_PER_CHIP;

    status = mod_pd_restricted_api->set_state(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, boot_cpu),
        false,
        MOD_PD_COMPOSITE_STATE(
            MOD_PD_LEVEL_2,
            0,
            MOD_PD_STATE_ON,
            MOD_PD_STATE_ON,
            MOD_PD_STATE_ON));

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            "[PLATFORM_SYSTEM] Failed to power up boot cpu,"
            " issuing cold reset to recover");
        /*
         * The power up request of boot cpu power domain has failed.
         * The warm reset cannot be completed and the system is now in
         * an unusable state. So perform a full system cold boot reset
         * by which the SCP will also be reset to recover from this
         * situation.
         */
        status =
            mod_pd_restricted_api->system_shutdown(MOD_PD_SYSTEM_COLD_RESET);
        fwk_assert(status == FWK_SUCCESS);
    }
}

static int platform_system_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
    int status;

    /* Event for checking power domain status */
    struct fwk_event_light check_pd_off_event = {
        .id = mod_platform_system_event_check_ppu_off,
        .target_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_PLATFORM_SYSTEM),
    };

    switch (fwk_id_get_event_idx(event->id)) {
    case MOD_PLATFORM_SYSTEM_CHECK_PD_OFF:
        status = check_power_off_all_cores();
        if (status != FWK_SUCCESS) {
            /*
             * Increment the retry count. The count is initialized in the warm
             * reset notification handler
             */
            platform_system_ctx.warm_reset_check_cnt++;
            if (platform_system_ctx.warm_reset_check_cnt >=
                WARM_RESET_MAX_RETRIES) {
                FWK_LOG_ERR(
                    "[PLATFORM_SYSTEM] warm reset retries reached "
                    "maximum attempts and failed!");
                fwk_assert(
                    platform_system_ctx.warm_reset_check_cnt <
                    WARM_RESET_MAX_RETRIES);
            }

            /*
             * Continue monitoring core PPUs until all the core power domains
             * are powered down.
             */
            status = fwk_put_event(&check_pd_off_event);
            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR(
                    "[PLATFORM_SYSTEM] Failed to send event, returned %d",
                    status);
            }
            fwk_assert(status == FWK_SUCCESS);
        } else {
            /*
             * All the CPU power domain are powered off. Start the process to
             * power on the first application core to complete the warm reboot
             * sequence.
             */
            boot_primary_core();
        }

        break; /* MOD_PLATFORM_SYSTEM_CHECK_PD_OFF */
    default:
        FWK_LOG_WARN(
            "[PLATFORM_SYSTEM] unrecognized event received, event ignored");
        status = FWK_E_PARAM;
    }

    return status;
}

int platform_system_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status;
    struct clock_notification_params *params;
    struct mod_pd_restricted_api *mod_pd_restricted_api;
    static unsigned int scmi_notification_count = 0;
    static bool sds_notification_received = false;
    uint8_t boot_cpu;
    fwk_id_t cpu_id;

    /* Event for checking power domain status */
    struct fwk_event_light check_pd_off_event = {
        .id = mod_platform_system_event_check_ppu_off,
        .target_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_PLATFORM_SYSTEM),
    };

    fwk_assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_MODULE));

    if (fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed)) {
        params = (struct clock_notification_params *)event->params;

        /*
         * Initialize primary core when the system is initialized for the first
         * time only
         */
        if (params->new_state == MOD_CLOCK_STATE_RUNNING) {
            FWK_LOG_INFO("[PLATFORM SYSTEM] Powering up the boot cpu...");

            boot_cpu = platform_calc_core_pos(
                           platform_system_ctx.config->primary_cpu_mpid) %
                MAX_PE_PER_CHIP;
            cpu_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, boot_cpu);

            mod_pd_restricted_api = platform_system_ctx.mod_pd_restricted_api;

            status = mod_pd_restricted_api->set_state(
                cpu_id,
                false,
                MOD_PD_COMPOSITE_STATE(
                    MOD_PD_LEVEL_2,
                    0,
                    MOD_PD_STATE_ON,
                    MOD_PD_STATE_ON,
                    MOD_PD_STATE_ON));

            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR(
                    "[PLATFORM SYSTEM] Failed to power up the boot cpu");
                return status;
            }

            /* Unsubscribe to the notification */
            return fwk_notification_unsubscribe(
                event->id, event->source_id, event->target_id);
        }

        return FWK_SUCCESS;
    } else if (fwk_id_is_equal(
                   event->id, mod_pd_notification_id_pre_warm_reset)) {
        /*
         * Notification handler for warm reset.
         *
         * On receiving warm reset request, the power domain (PD) module will
         * issue the warm reset notification. The notification is handled by
         * the platform system module. The notification handler requests PD to
         * power off all the CPU cores through the HAL API exposed by PD. After
         * requesting for power off for all the cores, the platform system
         * module will send an event to itself to check whether all core power
         * domains are powered off. After all the core power domains are powered
         * off, the event handler will then power up the boot CPU.
         */
        mod_pd_restricted_api = platform_system_ctx.mod_pd_restricted_api;
        power_off_all_cores();

        platform_system_ctx.warm_reset_check_cnt = 0;
        status = fwk_put_event(&check_pd_off_event);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                "[PLATFORM_SYSTEM] Failed to send PD power off check "
                "event, returned %d",
                status);
            FWK_LOG_ERR("[PLATFORM_SYSTEM] Issuing cold reboot to recover.");
            status = mod_pd_restricted_api->system_shutdown(
                MOD_PD_SYSTEM_COLD_RESET);
            fwk_assert(status == FWK_SUCCESS);
        }
    } else if (fwk_id_is_equal(
                   event->id, mod_scmi_notification_id_initialized)) {
        scmi_notification_count++;
    } else if (fwk_id_is_equal(
                   event->id, mod_sds_notification_id_initialized)) {
        sds_notification_received = true;
    } else {
        return FWK_E_PARAM;
    }

    if ((scmi_notification_count == FWK_ARRAY_SIZE(scmi_notification_table)) &&
        sds_notification_received) {
        status = messaging_stack_ready();
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                "[PLATFORM SYSTEM] Failed to update firmware feature "
                "availability flags in SDS");
            return status;
        }

        scmi_notification_count = 0;
        sds_notification_received = false;

        status = update_sds_reset_syndrome();
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR("[PLATFORM SYSTEM] Failed to update reset syndrome");
            return status;
        }

        status = update_primary_cpu_in_sds();
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                "[PLATFORM SYSTEM] Failed to update primary "
                "CPU number in SDS");
            return status;
        }

        status = update_isolated_mpid_list_in_sds();
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                "[PLATFORM SYSTEM] Failed to program isolated"
                " cpu list in SDS");
            return status;
        }
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_platform_system = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_PLATFORM_SYSTEM_API_COUNT,
    .event_count = (unsigned int)MOD_PLATFORM_SYSTEM_EVENT_COUNT,
    .init = platform_system_mod_init,
    .bind = platform_system_bind,
    .process_bind_request = platform_system_process_bind_request,
    .process_event = platform_system_process_event,
    .process_notification = platform_system_process_notification,
    .start = platform_system_start,
};
