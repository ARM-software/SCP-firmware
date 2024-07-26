/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     JUNO Reset Domain Driver
 *     Arm platforms don't have proper reset domains. However for completeness,
 *     this driver demonstrates reset functionality using Juno UART as an
 *     example reset domain peripheral. Only auto reset functionality is
 *     implemented as explicit assert/de-assert complicates functionality of
 *     UART peripheral.
 */

#include <juno_scc.h>
#include <system_mmap.h>

#include <mod_juno_reset_domain.h>
#include <mod_reset_domain.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

enum dev_state {
    DEVICE_STATE_NORMAL = 0,
    DEVICE_STATE_RESET = 1
};

struct juno_reset_dev_ctx {
    void *config;
    bool reset_state;
};

struct juno_reset_domain_module_ctx {
    const struct mod_reset_domain_drv_input_api *drv_input_api;
    fwk_id_t reset_domain_hal_id;
    struct juno_reset_dev_ctx *dev_ctx_table;
    uint32_t cookie;
};

static struct juno_reset_domain_module_ctx module_juno_reset_ctx;

/* Helper functions */
static int handle_uart_reset_set_state(struct juno_reset_dev_ctx *dev_ctx)
{
    int retry = 5;
    int status = FWK_E_DEVICE;

    struct mod_juno_reset_uart_config* uart_config =
        (struct mod_juno_reset_uart_config*)dev_ctx->config;

    if (dev_ctx->reset_state == DEVICE_STATE_RESET) {
        return FWK_E_STATE;
    }

    /* Reset UART device */
    dev_ctx->reset_state = DEVICE_STATE_RESET;
    *uart_config->reset_reg |= uart_config->reset_mask;

    /* Check if reset of the device has been taken place
     * On juno board unlikely we will see mutliple retries.
     */
    while (retry--) {
        if (*uart_config->reset_reg & uart_config->reset_mask) {
            /* We are only supporting auto reset and architecture reset
             * so clear juno UART reset.
             */
            *uart_config->reset_reg &= ~uart_config->reset_mask;
            status = FWK_SUCCESS;
            dev_ctx->reset_state = DEVICE_STATE_NORMAL;
            break;
        }
    }

    return status;
}

/*
 * Module APIs
 */
static int juno_set_reset_state(fwk_id_t dev_id,
                                enum mod_reset_domain_mode mode,
                                uint32_t reset_state,
                                uintptr_t cookie)
{
    int status;
    struct juno_reset_dev_ctx *dev_ctx;
    struct mod_reset_domain_autoreset_event_params *params;
    struct fwk_event autoreset_event = {
        .id = mod_reset_domain_autoreset_event_id,
        .target_id = fwk_module_id_reset_domain,
    };
    unsigned int domain_idx = fwk_id_get_element_idx(dev_id);

    if (domain_idx >= JUNO_RESET_DOMAIN_IDX_COUNT) {
        return FWK_E_PARAM;
    }

    dev_ctx = &module_juno_reset_ctx.dev_ctx_table[domain_idx];

    status = handle_uart_reset_set_state(dev_ctx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    params = (struct mod_reset_domain_autoreset_event_params *)
                 autoreset_event.params;
    params->dev_id = dev_id;
    params->reset_state = reset_state;
    params->cookie = cookie;
    fwk_put_event(&autoreset_event);

    return FWK_SUCCESS;
}

static struct mod_reset_domain_drv_api juno_reset_domain_drv_api = {
    .set_reset_state = juno_set_reset_state,
};

/*
 * Framework handlers
 */
static int juno_reset_domain_init(fwk_id_t module_id,
                                  unsigned int element_count,
                                  const void *data)
{
    /* This module supports only one reset device
     */
    fwk_assert(element_count == 1);
    module_juno_reset_ctx.dev_ctx_table = fwk_mm_calloc(element_count,
                                             sizeof(struct juno_reset_dev_ctx));

    return FWK_SUCCESS;
}

static int juno_reset_domain_element_init(fwk_id_t element_id,
                                          unsigned int sub_element_count,
                                          const void *data)
{
    struct juno_reset_dev_ctx* dev_ctx = NULL;

    dev_ctx = &module_juno_reset_ctx.dev_ctx_table[
        fwk_id_get_element_idx(element_id)];
    dev_ctx->config = (void*)data;

    return FWK_SUCCESS;
}

static int juno_reset_domain_bind(fwk_id_t id, unsigned int round)
{
   return FWK_SUCCESS;
}

static int juno_reset_domain_process_bind_request(fwk_id_t source_id,
                                                  fwk_id_t target_id,
                                                  fwk_id_t api_id,
                                                  const void **api)
{
    fwk_id_t mod_juno_reset_domain_api_id_driver =
        FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_RESET_DOMAIN,
                        MOD_JUNO_RESET_DOMAIN_API_IDX_DRIVER);

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT) ||
        !fwk_id_is_equal(api_id, mod_juno_reset_domain_api_id_driver) ||
        api == NULL) {
        return FWK_E_PARAM;
    }

    *api = &juno_reset_domain_drv_api;

    module_juno_reset_ctx.reset_domain_hal_id = source_id;

    return FWK_SUCCESS;
}

static int juno_reset_domain_start(fwk_id_t id)
{
    return FWK_SUCCESS;
}

struct fwk_module module_juno_reset_domain = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_JUNO_RESET_DOMAIN_API_IDX_COUNT,
    .init = juno_reset_domain_init,
    .element_init = juno_reset_domain_element_init,
    .bind = juno_reset_domain_bind,
    .process_bind_request = juno_reset_domain_process_bind_request,
    .start = juno_reset_domain_start,
};
