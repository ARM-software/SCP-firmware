/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP SCP to SCP I2C communications protocol driver
 */

/* The use of "primary" may not be in sync with older versions of TRM */

#include "config_clock.h"
#include "n1sdp_core.h"

#include <internal/n1sdp_scp2pcc.h>

#include <mod_cdns_i2c.h>
#include <mod_clock.h>
#include <mod_cmn600.h>
#include <mod_n1sdp_c2c_i2c.h>
#include <mod_n1sdp_dmc620.h>
#include <mod_n1sdp_pcie.h>
#include <mod_n1sdp_scp2pcc.h>
#include <mod_n1sdp_timer_sync.h>
#include <mod_power_domain.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Module definitions */
#define N1SDP_C2C_DATA_SIZE        8
#define CCIX_VC1_TC                7
#define CCIX_OPT_TLP_EN            1
#define CMN600_CCIX_LINK_ID        0

#define N1SDP_C2C_SUCCESS          0
#define N1SDP_C2C_ERROR            1

#define CORE_COUNT_PER_CHIP        4

#define C2C_PRIMARY_RETRY_DELAY_US UINT32_C(10000)
#define C2C_PRIMARY_RETRIES        10

/* Max Packet Size values */
#define CCIX_PROP_MAX_PACK_SIZE_128          0
#define CCIX_PROP_MAX_PACK_SIZE_256          1
#define CCIX_PROP_MAX_PACK_SIZE_512          2

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
static const char *const cmd_str[] = {
    [N1SDP_C2C_CMD_CHECK_SECONDARY] = "Check secondary alive",
    [N1SDP_C2C_CMD_PCIE_POWER_ON] = "PCIe power ON",
    [N1SDP_C2C_CMD_PCIE_PHY_INIT] = "PCIe PHY init",
    [N1SDP_C2C_CMD_PCIE_CTRL_INIT] = "PCIe controller init",
    [N1SDP_C2C_CMD_PCIE_LINK_TRAIN] = "PCIe link training",
    [N1SDP_C2C_CMD_PCIE_RC_SETUP] = "PCIe RC setup",
    [N1SDP_C2C_CMD_PCIE_VC1_CONFIG] = "PCIe VC1 setup",
    [N1SDP_C2C_CMD_PCIE_CCIX_CONFIG] = "PCIe CCIX setup",
    [N1SDP_C2C_CMD_CMN600_SET_CONFIG] = "CMN600 CCIX setup",
    [N1SDP_C2C_CMD_CMN600_XCHANGE_CREDITS] = "CMN600 exchange credits",
    [N1SDP_C2C_CMD_CMN600_ENTER_SYS_COHERENCY] =
        "CMN600 enter system coherency",
    [N1SDP_C2C_CMD_CMN600_ENTER_DVM_DOMAIN] = "CMN600 enter DVM domain",
    [N1SDP_C2C_CMD_GET_SLV_DDR_SIZE] = "Get secondary DDR size",
    [N1SDP_C2C_CMD_TIMER_SYNC] = "Sync timer",
    [N1SDP_C2C_CMD_POWER_DOMAIN_ON] = "Power domain ON",
    [N1SDP_C2C_CMD_POWER_DOMAIN_OFF] = "Power domain OFF",
    [N1SDP_C2C_CMD_POWER_DOMAIN_GET_STATE] = "Get power state",
    [N1SDP_C2C_CMD_SHUTDOWN_OR_REBOOT] = "Shutdown/Reboot",
};
#else
static const char *const cmd_str[] = { "" };
#endif

/* Module context */
struct n1sdp_c2c_ctx {
    /*  Pointer to module configuration */
    struct n1sdp_c2c_dev_config *config;

    /* I2C Controller API ID */
    struct mod_cdns_i2c_controller_api_polled *controller_api;

    /* I2C Target API ID */
    struct mod_cdns_i2c_target_api_irq *target_api;

    /* PCIe init API */
    struct n1sdp_pcie_init_api *pcie_init_api;

    /* CCIX config API */
    struct n1sdp_pcie_ccix_config_api *ccix_config_api;

    /* CMN600 config API */
    struct mod_cmn600_ccix_config_api *cmn600_api;

    /* Power domain module API */
    struct mod_pd_restricted_api *pd_api;

    /* DMC620 memory information API */
    struct mod_dmc620_mem_info_api *dmc620_api;

    /* Timer API */
    struct mod_timer_api *timer_api;

    /* Timer synchronization API */
    struct n1sdp_timer_sync_api *tsync_api;

    /* SCP to PCC communication API */
    struct mod_n1sdp_scp2pcc_api *scp2pcc_api;

    /* Chip ID */
    uint8_t chip_id;

    /* Multi chip mode flag */
    bool mc_mode;

    /* Identifier if secondary chip is alive or not */
    bool secondary_alive;

    /* Storage for secondary DDR size in GB */
    uint8_t secondary_ddr_size_gb;

    /* Storage for transmit data in primary mode */
    uint8_t primary_tx_data[N1SDP_C2C_DATA_SIZE];

    /* Storage for received data in primary mode */
    uint8_t primary_rx_data[N1SDP_C2C_DATA_SIZE];

    /* Storage for received data in secondary mode */
    uint8_t secondary_rx_data[N1SDP_C2C_DATA_SIZE];

    /* Storage for transmit data in secondary mode */
    uint8_t secondary_tx_data[N1SDP_C2C_DATA_SIZE];
};

static struct n1sdp_c2c_ctx n1sdp_c2c_ctx;

/*
 * Primary side protocol functions
 */
static int n1sdp_c2c_primary_tx_command(uint8_t cmd)
{
    int status;

    (void)cmd_str;
    FWK_LOG_INFO("[C2C] %s in secondary...", cmd_str[cmd]);

    n1sdp_c2c_ctx.primary_tx_data[0] = cmd;
    status = n1sdp_c2c_ctx.controller_api->write(
        n1sdp_c2c_ctx.config->i2c_id,
        n1sdp_c2c_ctx.config->target_addr,
        (char *)&n1sdp_c2c_ctx.primary_tx_data[0],
        N1SDP_C2C_DATA_SIZE,
        true);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[C2C] Error!");
        return status;
    }
    FWK_LOG_INFO("Done");

    return FWK_SUCCESS;
}

static int n1sdp_c2c_primary_rx_response(void)
{
    int status;

    FWK_LOG_INFO("[C2C] Waiting for response from secondary...");
    status = n1sdp_c2c_ctx.controller_api->read(
        n1sdp_c2c_ctx.config->i2c_id,
        n1sdp_c2c_ctx.config->target_addr,
        (char *)&n1sdp_c2c_ctx.primary_rx_data[0],
        N1SDP_C2C_DATA_SIZE);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[C2C] Error %d!", status);
        return status;
    }
    FWK_LOG_INFO("[C2C] Received");

    return FWK_SUCCESS;
}

static int n1sdp_c2c_check_remote(void)
{
    int status;

    if (n1sdp_c2c_ctx.chip_id == 0) {
        status = n1sdp_c2c_primary_tx_command(
            (uint8_t)N1SDP_C2C_CMD_CHECK_SECONDARY);
        if (status != FWK_SUCCESS) {
            n1sdp_c2c_ctx.secondary_alive = false;
            return FWK_E_DEVICE;
        }

        status = n1sdp_c2c_primary_rx_response();
        if (status != FWK_SUCCESS) {
            n1sdp_c2c_ctx.secondary_alive = false;
            return status;
        }

        n1sdp_c2c_ctx.secondary_alive = true;
        FWK_LOG_INFO("[C2C] Target Alive!");
        return FWK_SUCCESS;
    } else {
        status = n1sdp_c2c_ctx.target_api->read(
            n1sdp_c2c_ctx.config->i2c_id,
            &n1sdp_c2c_ctx.secondary_rx_data[0],
            N1SDP_C2C_DATA_SIZE);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[C2C] Error setting up I2C for reception!");
            return status;
        }
    }

    return FWK_SUCCESS;
}

static int n1sdp_c2c_multichip_run_command(uint8_t cmd, bool run_in_secondary)
{
    struct mod_cmn600_ccix_remote_node_config remote_config;
    int status;

    if (run_in_secondary) {
        status = n1sdp_c2c_primary_tx_command(cmd);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    (void)cmd_str;
    FWK_LOG_INFO("[C2C] %s in primary...", cmd_str[cmd]);

    switch (cmd) {
    case N1SDP_C2C_CMD_PCIE_POWER_ON:
        status = n1sdp_c2c_ctx.pcie_init_api->power_on(
            n1sdp_c2c_ctx.config->ccix_id);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[C2C] Error!");
            return status;
        }
        if (run_in_secondary) {
            status = n1sdp_c2c_primary_rx_response();
            if (status != FWK_SUCCESS) {
                return status;
            }
            if (n1sdp_c2c_ctx.primary_rx_data[0] != N1SDP_C2C_SUCCESS) {
                FWK_LOG_INFO("[C2C] Command failed in secondary!");
                return FWK_E_STATE;
            }
        }
        break;

    case N1SDP_C2C_CMD_PCIE_PHY_INIT:
        status = n1sdp_c2c_ctx.pcie_init_api->phy_init(
            n1sdp_c2c_ctx.config->ccix_id);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[C2C] Error!");
            return status;
        }
        if (run_in_secondary) {
            status = n1sdp_c2c_primary_rx_response();
            if (status != FWK_SUCCESS) {
                return status;
            }
            if (n1sdp_c2c_ctx.primary_rx_data[0] != N1SDP_C2C_SUCCESS) {
                FWK_LOG_INFO("[C2C] Command failed in secondary!");
                return FWK_E_STATE;
            }
        }
        break;

    case N1SDP_C2C_CMD_PCIE_CTRL_INIT:
        status = n1sdp_c2c_ctx.pcie_init_api->controller_init(
            n1sdp_c2c_ctx.config->ccix_id, false);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[C2C] Error!");
            return status;
        }
        if (run_in_secondary) {
            status = n1sdp_c2c_primary_rx_response();
            if (status != FWK_SUCCESS) {
                return status;
            }
            if (n1sdp_c2c_ctx.primary_rx_data[0] != N1SDP_C2C_SUCCESS) {
                FWK_LOG_INFO("[C2C] Command failed in secondary!");
                return FWK_E_STATE;
            }
        }
        break;

    case N1SDP_C2C_CMD_PCIE_LINK_TRAIN:
        status = n1sdp_c2c_ctx.pcie_init_api->link_training(
            n1sdp_c2c_ctx.config->ccix_id, false);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[C2C] Error!");
            return status;
        }
        if (run_in_secondary) {
            status = n1sdp_c2c_primary_rx_response();
            if (status != FWK_SUCCESS) {
                return status;
            }
            if (n1sdp_c2c_ctx.primary_rx_data[0] != N1SDP_C2C_SUCCESS) {
                FWK_LOG_INFO("[C2C] Command failed in secondary!");
                return FWK_E_STATE;
            }
        }
        break;

    case N1SDP_C2C_CMD_PCIE_RC_SETUP:
        status = n1sdp_c2c_ctx.pcie_init_api->rc_setup(
            n1sdp_c2c_ctx.config->ccix_id);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[C2C] Error!");
            return status;
        }
        if (run_in_secondary) {
            status = n1sdp_c2c_primary_rx_response();
            if (status != FWK_SUCCESS) {
                return status;
            }
            if (n1sdp_c2c_ctx.primary_rx_data[0] != N1SDP_C2C_SUCCESS) {
                FWK_LOG_INFO("[C2C] Command failed in secondary!");
                return FWK_E_STATE;
            }
        }
        break;

    case N1SDP_C2C_CMD_PCIE_VC1_CONFIG:
        status = n1sdp_c2c_ctx.pcie_init_api->vc1_setup(
            n1sdp_c2c_ctx.config->ccix_id, CCIX_VC1_TC);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[C2C] Error!");
            return status;
        }
        if (run_in_secondary) {
            status = n1sdp_c2c_primary_rx_response();
            if (status != FWK_SUCCESS) {
                return status;
            }
            if (n1sdp_c2c_ctx.primary_rx_data[0] != N1SDP_C2C_SUCCESS) {
                FWK_LOG_INFO("[C2C] Command failed in secondary!");
                return FWK_E_STATE;
            }
        }
        break;

    case N1SDP_C2C_CMD_PCIE_CCIX_CONFIG:
        status = n1sdp_c2c_ctx.ccix_config_api->enable_opt_tlp(
            CCIX_OPT_TLP_EN);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[C2C] Error!");
            return status;
        }
        if (run_in_secondary) {
            status = n1sdp_c2c_primary_rx_response();
            if (status != FWK_SUCCESS) {
                return status;
            }
            if (n1sdp_c2c_ctx.primary_rx_data[0] != N1SDP_C2C_SUCCESS) {
                FWK_LOG_INFO("[C2C] Command failed in secondary!");
                return FWK_E_STATE;
            }
        }
        break;

    case N1SDP_C2C_CMD_CMN600_SET_CONFIG:
        remote_config.remote_rnf_count = 2;
        remote_config.remote_sa_count = 0;
        remote_config.remote_ha_count = 1;
        remote_config.ccix_tc = CCIX_VC1_TC;
        remote_config.ccix_msg_pack_enable = false;
        remote_config.pcie_bus_num = 1;
        remote_config.ccix_link_id = 0;
        remote_config.ccix_opt_tlp = CCIX_OPT_TLP_EN;
        remote_config.smp_mode = true;
        remote_config.remote_ha_mmap_count = 1;
        remote_config.remote_ha_mmap[0].ha_id = 0x1;
        remote_config.remote_ha_mmap[0].base = (4ULL * FWK_TIB);
        remote_config.remote_ha_mmap[0].size = (4ULL * FWK_TIB);
        remote_config.ccix_max_packet_size = CCIX_PROP_MAX_PACK_SIZE_512;

        status = n1sdp_c2c_ctx.cmn600_api->set_config(&remote_config);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[C2C] Error!");
            return status;
        }
        if (run_in_secondary) {
            status = n1sdp_c2c_primary_rx_response();
            if (status != FWK_SUCCESS) {
                return status;
            }
            if (n1sdp_c2c_ctx.primary_rx_data[0] != N1SDP_C2C_SUCCESS) {
                FWK_LOG_INFO("[C2C] Command failed in secondary!");
                return FWK_E_STATE;
            }
        }
        break;

    case N1SDP_C2C_CMD_CMN600_XCHANGE_CREDITS:
        status = n1sdp_c2c_ctx.cmn600_api->exchange_protocol_credit(
            CMN600_CCIX_LINK_ID);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[C2C] Error!");
            return status;
        }
        if (run_in_secondary) {
            status = n1sdp_c2c_primary_rx_response();
            if (status != FWK_SUCCESS) {
                return status;
            }
            if (n1sdp_c2c_ctx.primary_rx_data[0] != N1SDP_C2C_SUCCESS) {
                FWK_LOG_INFO("[C2C] Command failed in secondary!");
                return FWK_E_STATE;
            }
        }
        break;

    case N1SDP_C2C_CMD_CMN600_ENTER_SYS_COHERENCY:
        status = n1sdp_c2c_ctx.cmn600_api->enter_system_coherency(
            CMN600_CCIX_LINK_ID);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[C2C] Error!");
            return status;
        }
        if (run_in_secondary) {
            status = n1sdp_c2c_primary_rx_response();
            if (status != FWK_SUCCESS) {
                return status;
            }
            if (n1sdp_c2c_ctx.primary_rx_data[0] != N1SDP_C2C_SUCCESS) {
                FWK_LOG_INFO("[C2C] Command failed in secondary!");
                return FWK_E_STATE;
            }
        }
        break;

    case N1SDP_C2C_CMD_CMN600_ENTER_DVM_DOMAIN:
        status = n1sdp_c2c_ctx.cmn600_api->enter_dvm_domain(
            CMN600_CCIX_LINK_ID);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[C2C] Error!");
            return status;
        }
        if (run_in_secondary) {
            status = n1sdp_c2c_primary_rx_response();
            if (status != FWK_SUCCESS) {
                return status;
            }
            if (n1sdp_c2c_ctx.primary_rx_data[0] != N1SDP_C2C_SUCCESS) {
                FWK_LOG_INFO("[C2C] Command failed in secondary!");
                return FWK_E_STATE;
            }
        }
        break;

    case N1SDP_C2C_CMD_GET_SLV_DDR_SIZE:
        if (run_in_secondary) {
            status = n1sdp_c2c_primary_rx_response();
            if (status != FWK_SUCCESS) {
                return status;
            }
            if (n1sdp_c2c_ctx.primary_rx_data[0] != N1SDP_C2C_SUCCESS) {
                FWK_LOG_INFO("[C2C] Command failed in secondary!");
                return FWK_E_STATE;
            }
            n1sdp_c2c_ctx.secondary_ddr_size_gb =
                n1sdp_c2c_ctx.primary_rx_data[1];
            FWK_LOG_INFO(
                "[C2C] Target DDR Size: %d GB",
                n1sdp_c2c_ctx.secondary_ddr_size_gb);
        }
        break;

    case N1SDP_C2C_CMD_TIMER_SYNC:
        status = n1sdp_c2c_ctx.tsync_api->primary_sync(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_N1SDP_TIMER_SYNC, 0));
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[C2C] Error!");
            return status;
        }
        if (run_in_secondary) {
            status = n1sdp_c2c_primary_rx_response();
            if (status != FWK_SUCCESS) {
                return status;
            }
            if (n1sdp_c2c_ctx.primary_rx_data[0] != N1SDP_C2C_SUCCESS) {
                FWK_LOG_INFO("[C2C] Command failed in secondary!");
                return FWK_E_STATE;
            }
        }
        break;

    default:
        FWK_LOG_INFO("[C2C] Unsupported command");
        return FWK_E_DEVICE;
    }

    return FWK_SUCCESS;
}

static int n1sdp_c2c_multichip_init(void)
{
    int status;

    status = n1sdp_c2c_multichip_run_command(N1SDP_C2C_CMD_PCIE_POWER_ON,
                                             true);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = n1sdp_c2c_multichip_run_command(N1SDP_C2C_CMD_PCIE_PHY_INIT,
                                             true);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = n1sdp_c2c_multichip_run_command(N1SDP_C2C_CMD_PCIE_CCIX_CONFIG,
                                             true);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = n1sdp_c2c_multichip_run_command(N1SDP_C2C_CMD_PCIE_CTRL_INIT,
                                             true);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = n1sdp_c2c_multichip_run_command(N1SDP_C2C_CMD_PCIE_LINK_TRAIN,
                                             true);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = n1sdp_c2c_multichip_run_command(N1SDP_C2C_CMD_PCIE_RC_SETUP,
                                             false);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = n1sdp_c2c_multichip_run_command(N1SDP_C2C_CMD_PCIE_VC1_CONFIG,
                                             false);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = n1sdp_c2c_multichip_run_command(N1SDP_C2C_CMD_CMN600_SET_CONFIG,
                                             true);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = n1sdp_c2c_multichip_run_command(
        N1SDP_C2C_CMD_CMN600_XCHANGE_CREDITS, true);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = n1sdp_c2c_multichip_run_command(
        N1SDP_C2C_CMD_CMN600_ENTER_SYS_COHERENCY, true);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = n1sdp_c2c_multichip_run_command(
        N1SDP_C2C_CMD_CMN600_ENTER_DVM_DOMAIN, true);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = n1sdp_c2c_multichip_run_command(N1SDP_C2C_CMD_GET_SLV_DDR_SIZE,
                                             true);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = n1sdp_c2c_multichip_run_command(N1SDP_C2C_CMD_TIMER_SYNC,
                                             true);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}

/*
 * Target side protocol functions
 */
static int n1sdp_c2c_wait_for_next_command(void)
{
    int status;

    status = n1sdp_c2c_ctx.target_api->read(
        n1sdp_c2c_ctx.config->i2c_id,
        &n1sdp_c2c_ctx.secondary_rx_data[0],
        N1SDP_C2C_DATA_SIZE);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[C2C] Error setting up read transfer!");
        return status;
    }

    return FWK_SUCCESS;
}

static int n1sdp_c2c_process_command(void)
{
    int status;
    uint8_t cmd;
    uint8_t rx_data[N1SDP_C2C_DATA_SIZE];
    uint32_t ddr_size_gb = 0;
    unsigned int state = 0;
    bool set_state_req_resp = false;
    struct mod_cmn600_ccix_remote_node_config remote_config;

    memcpy(rx_data, n1sdp_c2c_ctx.secondary_rx_data, N1SDP_C2C_DATA_SIZE);

    cmd = rx_data[0];

    switch (cmd) {
    case N1SDP_C2C_CMD_CHECK_SECONDARY:
        status = FWK_SUCCESS;
        break;

    case N1SDP_C2C_CMD_PCIE_POWER_ON:
        status = n1sdp_c2c_ctx.pcie_init_api->power_on(
            n1sdp_c2c_ctx.config->ccix_id);
        if (status != FWK_SUCCESS) {
            goto error;
        }
        break;

    case N1SDP_C2C_CMD_PCIE_PHY_INIT:
        status = n1sdp_c2c_ctx.pcie_init_api->phy_init(
            n1sdp_c2c_ctx.config->ccix_id);
        if (status != FWK_SUCCESS) {
            goto error;
        }
        break;

    case N1SDP_C2C_CMD_PCIE_CTRL_INIT:
        status = n1sdp_c2c_ctx.pcie_init_api->controller_init(
            n1sdp_c2c_ctx.config->ccix_id, true);
        if (status != FWK_SUCCESS) {
            goto error;
        }
        break;

    case N1SDP_C2C_CMD_PCIE_LINK_TRAIN:
        status = n1sdp_c2c_ctx.pcie_init_api->link_training(
            n1sdp_c2c_ctx.config->ccix_id, true);
        if (status != FWK_SUCCESS) {
            goto error;
        }
        break;

    case N1SDP_C2C_CMD_PCIE_CCIX_CONFIG:
        status = n1sdp_c2c_ctx.ccix_config_api->enable_opt_tlp(
            CCIX_OPT_TLP_EN);
        if (status != FWK_SUCCESS) {
            goto error;
        }
        break;

    case N1SDP_C2C_CMD_CMN600_SET_CONFIG:
        remote_config.remote_rnf_count = 2;
        remote_config.remote_sa_count = 0;
        remote_config.remote_ha_count = 1;
        remote_config.ccix_tc = CCIX_VC1_TC;
        remote_config.ccix_msg_pack_enable = false;
        remote_config.pcie_bus_num = 1;
        remote_config.ccix_link_id = 0;
        remote_config.ccix_opt_tlp = CCIX_OPT_TLP_EN;
        remote_config.smp_mode = true;
        remote_config.remote_ha_mmap_count = 1;
        remote_config.remote_ha_mmap[0].ha_id = 0x0;
        remote_config.remote_ha_mmap[0].base = 0;
        remote_config.remote_ha_mmap[0].size = (4ULL * FWK_TIB);
        status = n1sdp_c2c_ctx.cmn600_api->set_config(&remote_config);
        if (status != FWK_SUCCESS) {
            goto error;
        }
        break;

    case N1SDP_C2C_CMD_CMN600_XCHANGE_CREDITS:
        status = n1sdp_c2c_ctx.cmn600_api->exchange_protocol_credit(
            CMN600_CCIX_LINK_ID);
        if (status != FWK_SUCCESS) {
            goto error;
        }
        break;

    case N1SDP_C2C_CMD_CMN600_ENTER_SYS_COHERENCY:
        status = n1sdp_c2c_ctx.cmn600_api->enter_system_coherency(
            CMN600_CCIX_LINK_ID);
        if (status != FWK_SUCCESS) {
            goto error;
        }
        break;

    case N1SDP_C2C_CMD_CMN600_ENTER_DVM_DOMAIN:
        status = n1sdp_c2c_ctx.cmn600_api->enter_dvm_domain(
            CMN600_CCIX_LINK_ID);
        if (status != FWK_SUCCESS) {
            goto error;
        }
        break;

    case N1SDP_C2C_CMD_GET_SLV_DDR_SIZE:
        status = n1sdp_c2c_ctx.dmc620_api->get_mem_size_gb(&ddr_size_gb);
        if (status != FWK_SUCCESS) {
            goto error;
        }
        n1sdp_c2c_ctx.secondary_tx_data[1] = (uint8_t)ddr_size_gb;
        break;

    case N1SDP_C2C_CMD_POWER_DOMAIN_OFF:
        /*
         * rx_data[0] - Contains the C2C command
         * rx_data[1] - Contains secondary power domain ID
         * rx_data[2] - Contains secondary power domain type (core or cluster)
         *
         * The power domains set_state requests a response.
         *
         */
        set_state_req_resp = true;
        switch (rx_data[2]) {
        case MOD_PD_TYPE_CORE:
            status = n1sdp_c2c_ctx.pd_api->set_state(
                FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, rx_data[1]),
                MOD_PD_SET_STATE_REQ_RESP,
                MOD_PD_COMPOSITE_STATE(
                    MOD_PD_LEVEL_0, 0, 0, 0, MOD_PD_STATE_OFF));
            if (status != FWK_SUCCESS) {
                goto error;
            }
            break;

        case MOD_PD_TYPE_CLUSTER:
        case MOD_PD_TYPE_DEVICE_DEBUG:
            status = n1sdp_c2c_ctx.pd_api->set_state(
                FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, rx_data[1]),
                MOD_PD_SET_STATE_REQ_RESP,
                MOD_PD_STATE_OFF);
            if (status != FWK_SUCCESS) {
                goto error;
            }
            break;

        case MOD_PD_TYPE_SYSTEM:
            status = n1sdp_c2c_ctx.pd_api->set_state(
                FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, rx_data[1]),
                MOD_PD_SET_STATE_REQ_RESP,
                MOD_PD_STATE_OFF);
            if (status != FWK_SUCCESS) {
                goto error;
            }
            break;

        default:
            status = FWK_E_PARAM;
            goto error;
        }
        break;

    case N1SDP_C2C_CMD_POWER_DOMAIN_ON:
        /*
         * rx_data[0] - Contains the C2C command
         * rx_data[1] - Contains secondary power domain ID
         * rx_data[2] - Contains secondary power domain type (core or cluster)
         *
         * The power domains set_state requests a response.
         *
         */
        set_state_req_resp = true;
        switch (rx_data[2]) {
        case MOD_PD_TYPE_CORE:
            status = n1sdp_c2c_ctx.pd_api->set_state(
                FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, rx_data[1]),
                MOD_PD_SET_STATE_REQ_RESP,
                MOD_PD_COMPOSITE_STATE(
                    MOD_PD_LEVEL_2,
                    0,
                    MOD_PD_STATE_ON,
                    MOD_PD_STATE_ON,
                    MOD_PD_STATE_ON));
            if (status != FWK_SUCCESS) {
                goto error;
            }
            break;

        case MOD_PD_TYPE_CLUSTER:
        case MOD_PD_TYPE_DEVICE_DEBUG:
            status = n1sdp_c2c_ctx.pd_api->set_state(
                FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, rx_data[1]),
                MOD_PD_SET_STATE_REQ_RESP,
                MOD_PD_STATE_ON);
            if (status != FWK_SUCCESS) {
                goto error;
            }
            break;

        case MOD_PD_TYPE_SYSTEM:
            status = n1sdp_c2c_ctx.pd_api->set_state(
                FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, rx_data[1]),
                MOD_PD_SET_STATE_REQ_RESP,
                MOD_PD_STATE_ON);
            if (status != FWK_SUCCESS) {
                goto error;
            }
            break;

        default:
            status = FWK_E_PARAM;
            goto error;
        }
        break;

    case N1SDP_C2C_CMD_POWER_DOMAIN_GET_STATE:
        /*
         * rx_data[0] - Contains the C2C command
         * rx_data[1] - Contains secondary power domain ID
         */
        status = n1sdp_c2c_ctx.pd_api->get_state(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, rx_data[1]),
            &state);
        if (status != FWK_SUCCESS) {
            goto error;
        }

        n1sdp_c2c_ctx.secondary_tx_data[1] = (uint8_t)state;
        break;

    case N1SDP_C2C_CMD_TIMER_SYNC:
        status = n1sdp_c2c_ctx.tsync_api->secondary_sync(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_N1SDP_TIMER_SYNC, 0));
        if (status != FWK_SUCCESS) {
            goto error;
        }
        break;

    case N1SDP_C2C_CMD_SHUTDOWN_OR_REBOOT:
        /*
         * rx_data[0] - Contains the C2C command
         * rx_data[1] - Contains shutdown/reboot type
         */
        switch (rx_data[1]) {
        case MOD_PD_SYSTEM_SHUTDOWN:
            FWK_LOG_INFO("[C2C] Request PCC for system shutdown");
            status = n1sdp_c2c_ctx.scp2pcc_api->send(NULL, 0,
                                                     SCP2PCC_TYPE_SHUTDOWN);
            fwk_assert(status == FWK_SUCCESS);
            break;

        case MOD_PD_SYSTEM_COLD_RESET:
            FWK_LOG_INFO("[C2C] Request PCC for system reboot");
            status = n1sdp_c2c_ctx.scp2pcc_api->send(NULL, 0,
                                                     SCP2PCC_TYPE_REBOOT);
            fwk_assert(status == FWK_SUCCESS);
            break;

        default:
            FWK_LOG_INFO("[C2C] Unknown shutdown command!");
            status = FWK_E_PARAM;
            break;
        }
        NVIC_SystemReset();
        break;

    default:
        FWK_LOG_INFO("[C2C] Unsupported command %d", cmd);
        status = FWK_E_SUPPORT;
    }

error:
    if (status == FWK_SUCCESS) {
        if (set_state_req_resp) {
            /*
             * The power domain will take time to transition to the new state.
             * Listen for the event response from the power domain before
             * writing the reply to the remote requester.
             */
            return FWK_SUCCESS;
        } else {
            /* Not a set_state command which requires a response */
            n1sdp_c2c_ctx.secondary_tx_data[0] = N1SDP_C2C_SUCCESS;
        }
    } else {
        /* status != FWK_SUCCESS */
        n1sdp_c2c_ctx.secondary_tx_data[0] = N1SDP_C2C_ERROR;
    }

    status = n1sdp_c2c_ctx.target_api->write(
        n1sdp_c2c_ctx.config->i2c_id,
        &n1sdp_c2c_ctx.secondary_tx_data[0],
        N1SDP_C2C_DATA_SIZE);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[C2C] Error setting up write transfer!");
        return status;
    }

    return FWK_SUCCESS;
}

/*
 * Module APIs
 */

/*
 * Target information API
 */
static bool n1sdp_c2c_is_secondary_alive(void)
{
    return n1sdp_c2c_ctx.secondary_alive;
}

static int n1sdp_c2c_get_ddr_size_gb(uint8_t *size_gb)
{
    fwk_assert(size_gb != NULL);

    *size_gb = n1sdp_c2c_ctx.secondary_ddr_size_gb;

    return FWK_SUCCESS;
}

static const struct n1sdp_c2c_secondary_info_api secondary_info_api = {
    .is_secondary_alive = n1sdp_c2c_is_secondary_alive,
    .get_ddr_size_gb = n1sdp_c2c_get_ddr_size_gb,
};

/*
 * Power domain API
 */
static int n1sdp_c2c_pd_set_state(enum n1sdp_c2c_cmd cmd, uint8_t pd_id,
    uint8_t pd_type)
{
    int status;
    uint8_t retries;

    n1sdp_c2c_ctx.primary_tx_data[1] = pd_id;
    n1sdp_c2c_ctx.primary_tx_data[2] = pd_type;
    status = n1sdp_c2c_primary_tx_command((uint8_t)cmd);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /*
     * PD command in target will take some time to complete so primary
     * has to retry waiting for response from target.
     */
    retries = C2C_PRIMARY_RETRIES;
    do {
        status = n1sdp_c2c_primary_rx_response();
        if (status == FWK_SUCCESS) {
            break;
        }

        retries--;
        n1sdp_c2c_ctx.timer_api->delay(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
            C2C_PRIMARY_RETRY_DELAY_US);
    } while (retries != 0);

    if ((retries == 0) && (status != FWK_SUCCESS)) {
        return status;
    }

    if (n1sdp_c2c_ctx.primary_rx_data[0] != N1SDP_C2C_SUCCESS) {
        FWK_LOG_INFO("[C2C] PD request failed!");
        return FWK_E_STATE;
    }

    return FWK_SUCCESS;
}

static int n1sdp_c2c_pd_get_state(enum n1sdp_c2c_cmd cmd, uint8_t pd_id,
    unsigned int *state)
{
    int status;
    uint8_t retries;

    fwk_assert(state != NULL);

    n1sdp_c2c_ctx.primary_tx_data[1] = pd_id;
    status = n1sdp_c2c_primary_tx_command((uint8_t)cmd);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /*
     * PD command in target will take some time to complete so primary
     * has to retry waiting for response from target.
     */
    retries = C2C_PRIMARY_RETRIES;
    do {
        status = n1sdp_c2c_primary_rx_response();
        if (status == FWK_SUCCESS) {
            break;
        }
        retries--;

        n1sdp_c2c_ctx.timer_api->delay(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
            C2C_PRIMARY_RETRY_DELAY_US);
    } while (retries != 0);

    if ((retries == 0) && (status != FWK_SUCCESS)) {
        return status;
    }

    /*
     * primary_rx_data[0] contains return status code
     * primary_rx_data[1] contains the current PD state in secondary
     */
    if (n1sdp_c2c_ctx.primary_rx_data[0] != N1SDP_C2C_SUCCESS) {
        FWK_LOG_INFO("[C2C] PD request failed!");
        return FWK_E_STATE;
    }

    *state = n1sdp_c2c_ctx.primary_rx_data[1];

    return FWK_SUCCESS;
}

static int n1sdp_c2c_pd_shutdown_reboot(enum n1sdp_c2c_cmd cmd,
                                        enum mod_pd_system_shutdown type)
{
    n1sdp_c2c_ctx.primary_tx_data[1] = (uint8_t)type;
    /*
     * Send command to secondary. Don't wait for response as secondary will
     * trigger shutdown/reboot sequence of SoC and I2C may not be
     * available.
     */
    return n1sdp_c2c_primary_tx_command((uint8_t)cmd);
}

static const struct n1sdp_c2c_pd_api pd_api = {
    .set_state = n1sdp_c2c_pd_set_state,
    .get_state = n1sdp_c2c_pd_get_state,
    .shutdown_reboot = n1sdp_c2c_pd_shutdown_reboot,
};

/*
 * Framework Handlers
 */

static int n1sdp_c2c_init(fwk_id_t module_id, unsigned int unused,
    const void *data)
{
    if (data == NULL) {
        return FWK_E_PARAM;
    }

    n1sdp_c2c_ctx.config = (struct n1sdp_c2c_dev_config *)data;

    n1sdp_c2c_ctx.mc_mode = n1sdp_is_multichip_enabled();
    if (!n1sdp_c2c_ctx.mc_mode) {
        return FWK_SUCCESS;
    }

    n1sdp_c2c_ctx.chip_id = n1sdp_get_chipid();

    return FWK_SUCCESS;
}

static int n1sdp_c2c_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (!n1sdp_c2c_ctx.mc_mode) {
        return FWK_SUCCESS;
    }

    if (round == 0) {
        status = fwk_module_bind(n1sdp_c2c_ctx.config->ccix_id,
                                 FWK_ID_API(FWK_MODULE_IDX_N1SDP_PCIE,
                                            N1SDP_PCIE_API_IDX_PCIE_INIT),
                                 &n1sdp_c2c_ctx.pcie_init_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        status = fwk_module_bind(n1sdp_c2c_ctx.config->ccix_id,
                                 FWK_ID_API(FWK_MODULE_IDX_N1SDP_PCIE,
                                            N1SDP_PCIE_API_IDX_CCIX_CONFIG),
                                 &n1sdp_c2c_ctx.ccix_config_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_CMN600),
                                 FWK_ID_API(FWK_MODULE_IDX_CMN600,
                                            MOD_CMN600_API_IDX_CCIX_CONFIG),
                                 &n1sdp_c2c_ctx.cmn600_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        status = fwk_module_bind(fwk_module_id_power_domain,
                                 mod_pd_api_id_restricted,
                                 &n1sdp_c2c_ctx.pd_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_DMC620),
                                 FWK_ID_API(FWK_MODULE_IDX_N1SDP_DMC620,
                                            MOD_DMC620_API_IDX_MEM_INFO),
                                 &n1sdp_c2c_ctx.dmc620_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        status = fwk_module_bind(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                 FWK_ID_API(FWK_MODULE_IDX_TIMER,
                                            MOD_TIMER_API_IDX_TIMER),
                                 &n1sdp_c2c_ctx.timer_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        status = fwk_module_bind(
            FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_TIMER_SYNC),
            FWK_ID_API(FWK_MODULE_IDX_N1SDP_TIMER_SYNC,
                       N1SDP_TIMER_SYNC_API_IDX_TSYNC),
            &n1sdp_c2c_ctx.tsync_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_SCP2PCC),
                                 FWK_ID_API(FWK_MODULE_IDX_N1SDP_SCP2PCC, 0),
                                 &n1sdp_c2c_ctx.scp2pcc_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        if (n1sdp_c2c_ctx.chip_id == 0) {
            status = fwk_module_bind(
                n1sdp_c2c_ctx.config->i2c_id,
                FWK_ID_API(
                    FWK_MODULE_IDX_CDNS_I2C,
                    MOD_CDNS_I2C_API_CONTROLLER_POLLED),
                &n1sdp_c2c_ctx.controller_api);
            if (status != FWK_SUCCESS) {
                return status;
            }
        } else {
            status = fwk_module_bind(
                n1sdp_c2c_ctx.config->i2c_id,
                FWK_ID_API(
                    FWK_MODULE_IDX_CDNS_I2C, MOD_CDNS_I2C_API_TARGET_IRQ),
                &n1sdp_c2c_ctx.target_api);
            if (status != FWK_SUCCESS) {
                return status;
            }
        }
    }

    return FWK_SUCCESS;
}

static int n1sdp_c2c_process_bind_request(fwk_id_t requester_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case N1SDP_C2C_API_IDX_SECONDARY_INFO:
        *api = &secondary_info_api;
        break;
    case N1SDP_C2C_API_IDX_PD:
        *api = &pd_api;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int n1sdp_c2c_start(fwk_id_t id)
{
    int status;

    if (!n1sdp_c2c_ctx.mc_mode) {
        return FWK_SUCCESS;
    }

    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_SUCCESS;
    }

    status = fwk_notification_subscribe(
        mod_clock_notification_id_state_changed,
        FWK_ID_ELEMENT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_INTERCONNECT), id);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_notification_subscribe(
        mod_cdns_i2c_notification_id_target_rx,
        n1sdp_c2c_ctx.config->i2c_id,
        id);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return fwk_notification_subscribe(
        mod_cdns_i2c_notification_id_target_tx,
        n1sdp_c2c_ctx.config->i2c_id,
        id);
}

static int n1sdp_c2c_process_notification(const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status;

    if (fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed)) {
        status = n1sdp_c2c_check_remote();
        if (status == FWK_SUCCESS && (n1sdp_c2c_ctx.chip_id == 0)) {
            status = n1sdp_c2c_multichip_init();
            if (status != FWK_SUCCESS) {
                n1sdp_c2c_ctx.secondary_alive = false;
            }
        }
        /*
         * Unsubscribe from notification as C2C initialization will be done
         * only during cold boot and system suspend is not supported in N1SDP.
         */
        return fwk_notification_unsubscribe(event->id, event->source_id,
                                            event->target_id);
    } else if (fwk_id_is_equal(
                   event->id, mod_cdns_i2c_notification_id_target_rx)) {
        return n1sdp_c2c_process_command();
    } else if (fwk_id_is_equal(
                   event->id, mod_cdns_i2c_notification_id_target_tx)) {
        return n1sdp_c2c_wait_for_next_command();
    }

    return FWK_SUCCESS;
}
static int n1sdp_c2c_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status;
    unsigned int module_idx;

    module_idx = fwk_id_get_module_idx(event->source_id);
    if (module_idx == fwk_id_get_module_idx(fwk_module_id_power_domain)) {
        status = n1sdp_c2c_ctx.target_api->write(
            n1sdp_c2c_ctx.config->i2c_id,
            &n1sdp_c2c_ctx.secondary_tx_data[0],
            N1SDP_C2C_DATA_SIZE);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR("[C2C] Error setting up write transfer!");
            return status;
        }
        return FWK_SUCCESS;
    }
    return FWK_E_PARAM;
}

const struct fwk_module module_n1sdp_c2c = {
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .api_count = N1SDP_C2C_API_COUNT,
    .init = n1sdp_c2c_init,
    .bind = n1sdp_c2c_bind,
    .process_bind_request = n1sdp_c2c_process_bind_request,
    .process_notification = n1sdp_c2c_process_notification,
    .process_event = n1sdp_c2c_process_event,
    .start = n1sdp_c2c_start,
};
