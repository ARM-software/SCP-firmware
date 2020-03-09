/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_cdcel937.h"
#include "juno_clock.h"

#include <mod_clock.h>
#include <mod_i2c.h>
#include <mod_juno_cdcel937.h>
#include <mod_juno_hdlcd.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static const struct mod_juno_cdcel937_config *module_config;
static struct juno_cdcel937_module_ctx module_ctx;
static struct juno_cdcel937_dev_ctx *ctx_table;

/*
 * Get/Set rate requests can only be done for one device at a time. Thus this
 * buffer is used only by one device at a time.
 */
static uint8_t i2c_transmit[16];

/*
 * Helper functions
 */

static int pll_base_from_output_id(enum mod_juno_cdcel937_output_id output_id,
                                   uint8_t *base)
{
    /* Special case: no PLL for output Y1 */
    switch (output_id) {
    case MOD_JUNO_CDCEL937_OUTPUT_ID_Y2:
    case MOD_JUNO_CDCEL937_OUTPUT_ID_Y3:
        *base = JUNO_CDCEL937_BASE_PLL1_REG;
        return FWK_SUCCESS;

    case MOD_JUNO_CDCEL937_OUTPUT_ID_Y4:
    case MOD_JUNO_CDCEL937_OUTPUT_ID_Y5:
        *base = JUNO_CDCEL937_BASE_PLL2_REG;
        return FWK_SUCCESS;

    case MOD_JUNO_CDCEL937_OUTPUT_ID_Y6:
    case MOD_JUNO_CDCEL937_OUTPUT_ID_Y7:
        *base = JUNO_CDCEL937_BASE_PLL3_REG;
        return FWK_SUCCESS;

    default:
        return FWK_E_RANGE;

    }
}

static int output_type_from_output_id(
    enum mod_juno_cdcel937_output_id output_id,
    enum juno_cdcel937_output_type *type)
{
    switch (output_id) {
    case MOD_JUNO_CDCEL937_OUTPUT_ID_Y1:
        /* Y1 needs special handling */
        #if USE_OUTPUT_Y1
        *type = JUNO_CDCEL937_OUTPUT_TYPE_Y1;
        return FWK_SUCCESS;
        #else
        return FWK_E_SUPPORT;
        #endif

    case MOD_JUNO_CDCEL937_OUTPUT_ID_Y2:
    case MOD_JUNO_CDCEL937_OUTPUT_ID_Y4:
    case MOD_JUNO_CDCEL937_OUTPUT_ID_Y6:
        *type = JUNO_CDCEL937_OUTPUT_TYPE_LOW;
        return FWK_SUCCESS;

    case MOD_JUNO_CDCEL937_OUTPUT_ID_Y3:
    case MOD_JUNO_CDCEL937_OUTPUT_ID_Y5:
    case MOD_JUNO_CDCEL937_OUTPUT_ID_Y7:
        *type = JUNO_CDCEL937_OUTPUT_TYPE_HIGH;
        return FWK_SUCCESS;

    default:
        return FWK_E_RANGE;

    }
}
static int set_block_access_length(struct juno_cdcel937_dev_ctx *ctx,
                                   uint8_t length)
{
    int status;
    struct cfg_reg6 config6;

    if (length == 0)
        return FWK_E_RANGE;
    if (length > 127)
        /* 7 bit field limits maximum access length */
        return FWK_E_RANGE;

    config6.reg[0] = 0;
    write_field(config6.reg, CFG_REG6_BCOUNT, (uint8_t)length);
    write_field(config6.reg, CFG_REG6_EEWRITE, 0);

    i2c_transmit[0] = (JUNO_CDCEL937_TRANSFER_MODE_BYTE |
                      ((JUNO_CDCEL937_BASE_CFG_REG + 0x6) &
                      JUNO_CDCEL937_I2C_BYTE_OFFSET_MASK));

    i2c_transmit[1] = config6.reg[0];

    status = module_ctx.i2c_api->transmit_as_master(module_config->i2c_hal_id,
        ctx->config->slave_address, i2c_transmit, 2);
    if (status != FWK_PENDING)
        return FWK_E_DEVICE;

    return status;
}

static int write_configuration(struct juno_cdcel937_dev_ctx *ctx,
                               uint8_t base,
                               struct pll_cfg_reg *config)
{
    int status;

    if (base > 0x40)
        return FWK_E_RANGE;

    i2c_transmit[0] = (JUNO_CDCEL937_TRANSFER_MODE_BLOCK |
                      (base & JUNO_CDCEL937_I2C_BYTE_OFFSET_MASK));
    i2c_transmit[1] = 8;

    /*
     * When writing back the structure, we want to skip the 1-byte RESERVED
     * field at the beginning. This was used earlier because the chip sends
     * the transaction length (in bytes) at the beginning of each read.
     */
    memcpy(&i2c_transmit[2], &(config->reg[1]), 8);

    status = module_ctx.i2c_api->transmit_as_master(module_config->i2c_hal_id,
        ctx->config->slave_address, i2c_transmit, 10);
    if (status != FWK_PENDING)
        return FWK_E_DEVICE;

    return status;
}

static int read_configuration(struct juno_cdcel937_dev_ctx *ctx,
                              uint8_t base,
                              struct pll_cfg_reg *config)
{
    int status;
    const uint8_t block_access_length = 8;

    if (base > 0x40)
        return FWK_E_RANGE;

    i2c_transmit[0] = (JUNO_CDCEL937_TRANSFER_MODE_BLOCK |
                   (base & JUNO_CDCEL937_I2C_BYTE_OFFSET_MASK));

    /* Returned data is preceded by a 1 byte header */
    status = module_ctx.i2c_api->transmit_then_receive_as_master(
        module_config->i2c_hal_id, ctx->config->slave_address, i2c_transmit,
        config->reg, 1, block_access_length + 1);
    if (status != FWK_PENDING)
        return FWK_E_DEVICE;

    return status;
}

#if USE_OUTPUT_Y1
static int read_configuration_y1(struct juno_cdcel937_dev_ctx *ctx,
                                 struct cfg_reg_y1 *config)
{
    int status;
    const uint8_t block_access_length = 2;

    i2c_transmit[0] = (JUNO_CDCEL937_TRANSFER_MODE_BLOCK |
        ((JUNO_CDCEL937_BASE_CFG_REG + 0x2) &
            JUNO_CDCEL937_I2C_BYTE_OFFSET_MASK));

    /* Returned data is preceded by a 1 byte header */
    status = module_ctx.i2c_api->transmit_then_receive_as_master(
        module_config->i2c_hal_id, ctx->config->slave_address, i2c_transmit,
        config->reg, 1, block_access_length + 1);
    if (status != FWK_PENDING)
        return FWK_E_DEVICE;

    return status;
}
#endif

static int get_preset_low_precision(uint64_t rate,
                                    struct juno_clock_preset *preset)
{
    uint32_t freq;

    freq = (uint32_t)rate / (uint32_t)FWK_MHZ;

    fwk_assert(freq <= 230);

    preset->PDIV = UINT32_C(230) / freq;
    preset->N = freq * preset->PDIV;
    preset->M = 24;

    return FWK_SUCCESS;
}

static int search_preset(struct juno_cdcel937_dev_ctx *ctx,
                         uint64_t rate,
                         struct juno_clock_preset *preset)
{
    unsigned int i;
    bool lookup_hit = false;

    /*
     * Check if we can use a value from the lookup table for extra precision
     */
    i = ctx->config->lookup_table_count;
    while (i-- > 0) {
       if (ctx->config->lookup_table[i].rate_hz == rate) {
            *preset = ctx->config->lookup_table[i].preset;
            lookup_hit = true;
            break;
        }
    }
    if (!lookup_hit) {
        /*
         * Set clock via run-time calculation. Low precision.
         */
        return get_preset_low_precision(rate, preset);
    }

    return FWK_SUCCESS;
}

static int get_rounded_rate(struct juno_cdcel937_dev_ctx *ctx,
                            uint64_t rate,
                            enum mod_clock_round_mode round_mode,
                            uint64_t *rounded_rate)
{
    uint64_t rounded_up_rate;
    uint64_t rounded_down_rate;

    if (ctx->config->rate_type != MOD_CLOCK_RATE_TYPE_CONTINUOUS)
        return FWK_E_SUPPORT;

    rounded_down_rate = FWK_ALIGN_PREVIOUS(rate, ctx->config->min_step);
    rounded_up_rate = FWK_ALIGN_NEXT(rate, ctx->config->min_step);

    switch (round_mode) {
    case MOD_CLOCK_ROUND_MODE_NONE:
        *rounded_rate = rate;
        break;

    case MOD_CLOCK_ROUND_MODE_NEAREST:
        /* Select the rounded rate that is closest to the given rate */
        if ((*rounded_rate % ctx->config->min_step) == 0)
            *rounded_rate = rate;
        else if ((rate - rounded_down_rate) > (rounded_up_rate - rate))
            *rounded_rate = rounded_up_rate;
        else
            *rounded_rate = rounded_down_rate;
        break;

    case MOD_CLOCK_ROUND_MODE_DOWN:
        *rounded_rate = rounded_down_rate;
        break;

    case MOD_CLOCK_ROUND_MODE_UP:
        *rounded_rate = rounded_up_rate;
        break;

    default:
        return FWK_E_SUPPORT;

    }

    /* Control that the obtained rate uses the correct precision. */
    if ((*rounded_rate % ctx->config->min_step) != 0)
        return FWK_E_RANGE;

    return FWK_SUCCESS;
}

static int set_rate_read_pll_config(struct juno_cdcel937_dev_ctx *ctx)
{
    int status;
    uint8_t base_address;
    enum juno_cdcel937_output_type type;

    status = output_type_from_output_id(ctx->config->output_id, &type);
    if (status != FWK_SUCCESS)
        return FWK_E_PARAM;

    #if USE_OUTPUT_Y1
    if (type == JUNO_CDCEL937_OUTPUT_TYPE_Y1)
        return FWK_E_PARAM;
    #endif

    status = pll_base_from_output_id(ctx->config->output_id, &base_address);
    if (status != FWK_SUCCESS)
        return FWK_E_PARAM;

    /* The first 4 bytes of the PLL config register are not needed */
    base_address += 0x4;
    status = read_configuration(ctx, base_address, &ctx->pll_config);
    if ((status != FWK_PENDING) && (status != FWK_SUCCESS))
        return FWK_E_DEVICE;

    return status;
}

static int set_rate_write_pll_config(struct juno_cdcel937_dev_ctx *ctx)
{
    int status;
    struct pll_cfg_reg pll_config;
    enum juno_cdcel937_output_type type;
    uint8_t base_address;
    struct juno_clock_preset preset;
    unsigned int fvco, V, P, Nd, Q, R;
    unsigned int pdiv, N, M;

    pll_config = ctx->pll_config;

    status = output_type_from_output_id(ctx->config->output_id, &type);
    if (status != FWK_SUCCESS)
        return FWK_E_PARAM;

    if (ctx->config->lookup_table_count == 0)
        status = get_preset_low_precision(ctx->rate, &preset);
    else if (ctx->index < 0)
        status = search_preset(ctx, ctx->rate, &preset);
    else {
        if ((unsigned int)ctx->index >= ctx->config->lookup_table_count)
            return FWK_E_PARAM;

        preset = ctx->config->lookup_table[ctx->index].preset;
    }

    if (status != FWK_SUCCESS)
        return FWK_E_PARAM;

    M = preset.M;
    N = preset.N;
    pdiv = preset.PDIV;

    fwk_assert(M != 0);

    fvco = (ctx->config->xin_mhz * N) / M;
    if ((fvco < JUNO_CDCEL937_PLL_MIN_FREQ_MHZ) ||
        (fvco > JUNO_CDCEL937_PLL_MAX_FREQ_MHZ)) {
        return FWK_E_PARAM;
    }

    /* Calculate P, P = 4 - int(log2 N/M) */
    V = N / M;
    if (V < 2)
        P = 4;
    else if (V < 3)
        P = 3;
    else if (V < 6)
        P = 2;
    else if (V < 12)
        P = 1;
    else
        P = 0;

    /* Calculate N', N' = N * 2^P */
    Nd = N * (1 << P);

    /* Calculate Q, Q = int(N' / M) */
    Q = Nd / M;

    /* Calculate R, R = N' - (M  * Q) */
    R = Nd - (M * Q);

    write_field(pll_config.reg, PLL_CFG_REG_N_HIGH, N >> 4);
    write_field(pll_config.reg, PLL_CFG_REG_N_LOW, N);
    write_field(pll_config.reg, PLL_CFG_REG_R_HIGH, R >> 5);
    write_field(pll_config.reg, PLL_CFG_REG_R_LOW, R);
    write_field(pll_config.reg, PLL_CFG_REG_Q_HIGH, Q >> 3);
    write_field(pll_config.reg, PLL_CFG_REG_Q_LOW, Q);
    write_field(pll_config.reg, PLL_CFG_REG_P, P);

    if (type == JUNO_CDCEL937_OUTPUT_TYPE_LOW)
        /* Even outputs: Y2, Y4, Y6 */
        write_field(pll_config.reg, PLL_CFG_REG_PDIV_X, pdiv);
    else
        /* Odd outputs: Y3, Y5, Y7 */
        write_field(pll_config.reg, PLL_CFG_REG_PDIV_Y, pdiv);

    /* VCO range may need updating when fVCO changes */
    if (fvco < 125)
        write_field(pll_config.reg, PLL_CFG_REG_VCO_RANGE,
                   JUNO_CDCEL937_VCO_RANGE_LOWEST);
    else if (fvco < 150)
        write_field(pll_config.reg, PLL_CFG_REG_VCO_RANGE,
                   JUNO_CDCEL937_VCO_RANGE_LOW);
    else if (fvco < 175)
        write_field(pll_config.reg, PLL_CFG_REG_VCO_RANGE,
                   JUNO_CDCEL937_VCO_RANGE_HIGH);
    else
        write_field(pll_config.reg, PLL_CFG_REG_VCO_RANGE,
                   JUNO_CDCEL937_VCO_RANGE_HIGHEST);

    status = pll_base_from_output_id(ctx->config->output_id, &base_address);
    if (status != FWK_SUCCESS)
        return FWK_E_PARAM;

    /* The first 4 bytes of the PLL config register are not needed */
    base_address += 0x4;

    /* Write back the modified structure */
    status = write_configuration(ctx, base_address, &pll_config);
    if ((status != FWK_PENDING) && (status != FWK_SUCCESS))
        return FWK_E_DEVICE;

    return status;
}

static int create_set_rate_request(fwk_id_t clock_id,
                                   struct juno_cdcel937_dev_ctx *ctx,
                                   uint64_t rate,
                                   int index)
{
    int status;
    struct fwk_event event;

    if (rate < JUNO_CDCEL937_OUTPUT_MIN_FREQ)
        return FWK_E_RANGE;
    if (rate > JUNO_CDCEL937_OUTPUT_MAX_FREQ)
        return FWK_E_RANGE;

    /*
     * If the clock rate has already the wanted value the setting process is
     * skipped.
     */
    if (ctx->rate_hz == rate)
        return FWK_SUCCESS;

    if (ctx->state != JUNO_CDCEL937_DEVICE_IDLE)
        return FWK_E_BUSY;

    event = (struct fwk_event) {
        .target_id = clock_id,
        .id = juno_cdcel937_event_id_set_rate,
    };

    ctx->rate = rate;
    ctx->index = index;

    status = fwk_thread_put_event(&event);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    ctx->state = JUNO_CDCEL937_DEVICE_SET_RATE_SET_BLOCK_ACCESS_LENGTH;

    return FWK_PENDING;
}

#if USE_OUTPUT_Y1
static int get_rate_y1_set_block_access_length(
    struct juno_cdcel937_dev_ctx *ctx)
{
    int status;
    const uint8_t block_access_length = 2;

    status = set_block_access_length(ctx, block_access_length);
    if ((status != FWK_PENDING) && (status != FWK_SUCCESS))
        return FWK_E_DEVICE;

    return status;
}

static int get_rate_y1_read_y1_config(struct juno_cdcel937_dev_ctx *ctx)
{
    int status;

    status = read_configuration_y1(ctx, &ctx->y1_config);
    if ((status != FWK_PENDING) && (status != FWK_SUCCESS))
        return FWK_E_DEVICE;

    return status;
}

static int get_rate_y1_check_pdiv(struct juno_cdcel937_dev_ctx *ctx)
{
    int status;
    unsigned int pdiv;
    const uint8_t block_access_length = 2;


    /* Is Y1 being fed by XIn or by PLL1? */
    if (read_field(ctx->y1_config.reg, CFG_REG_Y1_CLOCK_SOURCE) ==
                  JUNO_CDCEL937_Y1_CLK_SOURCE_PLL) {
        status = set_block_access_length(ctx, block_access_length);
        if ((status != FWK_PENDING) && (status != FWK_SUCCESS))
            return FWK_E_DEVICE;
        return status;
    } else {
        /* Y1 divider has 10-bit precision, all others have 7-bit */
        pdiv = read_field(ctx->y1_config.reg, CFG_REG_Y1_PDIV_LOW) |
                         ((uint16_t)read_field(ctx->y1_config.reg,
                         CFG_REG_Y1_PDIV_HIGH) << 8);
        fwk_assert(pdiv != 0);
        ctx->rate = (ctx->config->xin_mhz / pdiv) * FWK_MHZ;
        return FWK_SUCCESS;
    }
}

static int get_rate_y1_read_pll_config(struct juno_cdcel937_dev_ctx *ctx)
{
    int status;
    uint8_t base_address;

    base_address = JUNO_CDCEL937_BASE_PLL1_REG + 0x4;
    status = read_configuration(ctx, base_address, &ctx->pll_config);
    if ((status != FWK_PENDING) && (status != FWK_SUCCESS))
        return FWK_E_DEVICE;

    return status;
}

static int get_rate_y1_calc(struct juno_cdcel937_dev_ctx *ctx)
{
    unsigned int n, pdiv;

    /*
     * To determine the current frequency of the output we need to request:
     * 1) The state of the output mux
     * 2) The state of the PLL mux (bypassed or not)
     * 3) The output divider settings
     * 4) The PLL frequency (fVCO)
     *
     * This driver does not currently support the 'frequency selection' feature
     * of the chip which supports an alternate configuration (FS_0 / FS_1). We
     * make the assumption that the chip is always using the primary
     * configurations since the Juno platform doesn't implement this feature.
     *
     * Calculate the output frequency using the following formula:
     * fOut = (fIn / pdiv) * (N / M)
     * At the start up M = fIn / Mhz so the equation becomes:
     * fOut = (fIn / pdiv) * (N / fIn) * MHz
     * which can be further simplified:
     * fOut = (N * MHz) / pdiv
     */
    pdiv = read_field(ctx->y1_config.reg, CFG_REG_Y1_PDIV_LOW) |
        ((uint16_t)read_field(ctx->y1_config.reg, CFG_REG_Y1_PDIV_HIGH) << 8);
    n = read_field(ctx->pll_config.reg, PLL_CFG_REG_N_LOW) |
        ((uint16_t)read_field(ctx->pll_config.reg, PLL_CFG_REG_N_HIGH) << 4);

    fwk_assert(pdiv != 0);
    ctx->rate = (n * FWK_MHZ) / pdiv;

    return FWK_SUCCESS;
}
#endif

static int rate_set_block_access_length(struct juno_cdcel937_dev_ctx *ctx)
{
    int status;
    const uint8_t block_access_length = 8;

    status = set_block_access_length(ctx, block_access_length);
    if (status != FWK_PENDING)
        return FWK_E_DEVICE;

    return status;
}

static int get_rate_read_pll_config(struct juno_cdcel937_dev_ctx *ctx)
{
    int status;
    uint8_t base_address;

    status = pll_base_from_output_id(ctx->config->output_id, &base_address);
    if (status != FWK_SUCCESS)
        return status;

    /* The first 4 bytes of the PLL config register are not needed */
    base_address += 0x4;
    status = read_configuration(ctx, base_address, &ctx->pll_config);
    if ((status != FWK_PENDING) && (status != FWK_SUCCESS))
        return FWK_E_DEVICE;

    return status;
}

static int get_rate_calc(struct juno_cdcel937_dev_ctx *ctx)
{
    int status;
    unsigned int n, pdiv;
    enum juno_cdcel937_output_type type;

    status = output_type_from_output_id(ctx->config->output_id, &type);
    if (status != FWK_SUCCESS)
        return status;

    if (type == JUNO_CDCEL937_OUTPUT_TYPE_LOW) {
        /* Even outputs: Y2, Y4, Y6 */
        pdiv = read_field(ctx->pll_config.reg, PLL_CFG_REG_PDIV_X);
    } else {
        /* Odd outputs: Y3, Y5, Y7 */
        pdiv = read_field(ctx->pll_config.reg, PLL_CFG_REG_PDIV_Y);
    }

    /*
     * To determine the current frequency of the output we need to request:
     * 1) The state of the output mux
     * 2) The state of the PLL mux (bypassed or not)
     * 3) The output divider settings
     * 4) The PLL frequency (fVCO)
     *
     * This driver does not currently support the 'frequency selection' feature
     * of the chip which supports an alternate configuration (FS_0 / FS_1). We
     * make the assumption that the chip is always using the primary
     * configurations since the MCC doesn't implement this feature either.
     *
     * Calculate the output frequency using the following formula:
     * fOut = (fIn / pdiv) * (N / M)
     * At the start up M = fIn / Mhz so the equation becomes:
     * fOut = (fIn / pdiv) * (N / fIn) * MHz
     * which can be further simplified:
     * fOut = (N * MHz) / pdiv
     */
    n = read_field(ctx->pll_config.reg, PLL_CFG_REG_N_LOW) |
        ((uint16_t)read_field(ctx->pll_config.reg, PLL_CFG_REG_N_HIGH) << 4);

    fwk_assert(pdiv != 0);
    ctx->rate = (n * FWK_MHZ) / pdiv;

    return FWK_SUCCESS;
}

static int create_get_rate_request(fwk_id_t clock_id,
                                   struct juno_cdcel937_dev_ctx *ctx)
{
    int status;
    struct fwk_event event;

    if (ctx->state != JUNO_CDCEL937_DEVICE_IDLE)
            return FWK_E_BUSY;

    event = (struct fwk_event) {
        .target_id = clock_id,
        .id = juno_cdcel937_event_id_get_rate,
    };

    status = fwk_thread_put_event(&event);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    ctx->state = JUNO_CDCEL937_DEVICE_GET_RATE_SET_BLOCK_ACCESS_LENGTH;

    return FWK_PENDING;
}

/*
 * Clock HAL Driver API
 */
static int juno_cdcel937_set_rate(fwk_id_t clock_id,
                                  uint64_t rate,
                                  enum mod_clock_round_mode round_mode)
{
    int status;
    uint64_t rounded_rate = rate;
    struct juno_cdcel937_dev_ctx *ctx;

    ctx = &ctx_table[fwk_id_get_element_idx(clock_id)];

    if (ctx->config->rate_type == MOD_CLOCK_RATE_TYPE_CONTINUOUS) {
        status = get_rounded_rate(ctx, rate, round_mode, &rounded_rate);
        if (status != FWK_SUCCESS)
            return status;
    }

    /*
     * The index is given a negative value as it is invalid. The rate is
     * directly given and does not need to be retrieved from a table.
     */
    return create_set_rate_request(clock_id, ctx, rounded_rate, -1);
}

static int juno_cdcel937_get_rate(fwk_id_t clock_id,
                                  uint64_t *rate)
{
    int status;
    struct juno_cdcel937_dev_ctx *ctx;

    ctx = &ctx_table[fwk_id_get_element_idx(clock_id)];

    if (ctx->rate_set == false) {
        status = create_get_rate_request(clock_id, ctx);
        if ((status != FWK_PENDING) && (status != FWK_SUCCESS))
            return FWK_E_DEVICE;
        return status;
    }

    *rate = ctx->rate_hz;

    return FWK_SUCCESS;
}

static int juno_cdcel937_get_rate_from_index(fwk_id_t clock_id,
                                             unsigned int rate_index,
                                             uint64_t *rate)
{
    struct juno_cdcel937_dev_ctx *ctx;

    ctx = &ctx_table[fwk_id_get_element_idx(clock_id)];

    if (rate_index >= ctx->config->lookup_table_count)
        return FWK_E_PARAM;

    *rate = ctx->config->lookup_table[rate_index].rate_hz;

    return FWK_SUCCESS;
}

static int juno_cdcel937_set_state(fwk_id_t clock_id,
                                   enum mod_clock_state state)
{
    if (state != MOD_CLOCK_STATE_RUNNING)
        return FWK_E_SUPPORT;

    /*
     * The clock is always on, only allow state to be set RUNNING
     */

    return FWK_SUCCESS;
}

static int juno_cdcel937_get_state(fwk_id_t clock_id,
                                   enum mod_clock_state *state)
{
    /*
     * It is unlikely that outputs will be disabled as, due to the design of
     * the device outputs, disabling one clock could unintentionally
     * disable another. Therefore the state of all outputs will be permanently
     * enabled.
     */
    *state = MOD_CLOCK_STATE_RUNNING;

    return FWK_SUCCESS;
}

static int juno_cdcel937_get_range(fwk_id_t clock_id,
                                   struct mod_clock_range *range)
{
    unsigned int last_idx;
    struct juno_cdcel937_dev_ctx *ctx;

    ctx = &ctx_table[fwk_id_get_element_idx(clock_id)];

    if (ctx->config->rate_type == MOD_CLOCK_RATE_TYPE_DISCRETE) {
        last_idx = ctx->config->lookup_table_count - 1;
        range->min = ctx->config->lookup_table[0].rate_hz;
        range->max = ctx->config->lookup_table[last_idx].rate_hz;
        range->rate_count = ctx->config->lookup_table_count;
    } else {
        range->min = ctx->config->min_rate;
        range->max = ctx->config->max_rate;
        range->step = ctx->config->min_step;
    }

    range->rate_type = ctx->config->rate_type;

    return FWK_SUCCESS;
}

/* HDLCD driver API for shared I2C device */
static int juno_cdcel937_set_rate_from_index(fwk_id_t clock_id,
                                             int index)
{
    uint64_t rate;
    struct juno_cdcel937_dev_ctx *ctx;

    ctx = &ctx_table[fwk_id_get_element_idx(clock_id)];

    if ((index < 0) || (unsigned int)index >= ctx->config->lookup_table_count)
        return FWK_E_PARAM;

    rate = ctx->config->lookup_table[index].rate_hz;

    return create_set_rate_request(clock_id, ctx, rate, index);
}

static const struct mod_clock_drv_api clock_driver_api = {
    .set_rate = juno_cdcel937_set_rate,
    .get_rate = juno_cdcel937_get_rate,
    .get_rate_from_index = juno_cdcel937_get_rate_from_index,
    .set_state = juno_cdcel937_set_state,
    .get_state = juno_cdcel937_get_state,
    .get_range = juno_cdcel937_get_range,
};

/* API for HDLCD driver to share I2C device */
static const struct mod_juno_hdlcd_drv_api hdlcd_driver_api = {
    .set_rate_from_index = juno_cdcel937_set_rate_from_index,
};

/*
 * Framework handlers
 */
static int juno_cdcel937_init(fwk_id_t module_id,
                              unsigned int element_count,
                              const void *data)
{
    ctx_table = fwk_mm_calloc(element_count, sizeof(*ctx_table));

    module_config = data;

    return FWK_SUCCESS;
}

static int juno_cdcel937_dev_init(fwk_id_t element_id,
                                  unsigned int sub_element_count,
                                  const void *data)
{
    struct juno_cdcel937_dev_ctx *ctx;
    const struct mod_juno_cdcel937_dev_config *config = data;

    /*
     * It is expected that a discrete rate type has an associated lookup table
     */
    fwk_assert((config->rate_type == MOD_CLOCK_RATE_TYPE_CONTINUOUS)
        || ((config->lookup_table_count != 0) &&
            (config->lookup_table != NULL)));

    ctx = &ctx_table[fwk_id_get_element_idx(element_id)];

    ctx->config = config;
    ctx->rate_hz = -1;
    ctx->rate_set = false;
    ctx->state = JUNO_CDCEL937_DEVICE_IDLE;

    return FWK_SUCCESS;
}

static int juno_cdcel937_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct juno_cdcel937_dev_ctx *ctx;

    /*
     * Only bind in first round of calls
     */
    if (round > 0)
        return FWK_SUCCESS;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        /* Bind to I2C HAL */
        status = fwk_module_bind(module_config->i2c_hal_id,
                                 mod_i2c_api_id_i2c,
                                 &module_ctx.i2c_api);
        if (status != FWK_SUCCESS)
            return FWK_E_HANDLER;

        return FWK_SUCCESS;
    }

    ctx = &ctx_table[fwk_id_get_element_idx(id)];

    /* Bind to clock HAL */
    status = fwk_module_bind(ctx->config->clock_hal_id,
                             ctx->config->clock_api_id,
                             &ctx->driver_response_api);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

static int juno_cdcel937_process_bind_request(fwk_id_t source_id,
                                              fwk_id_t target_id,
                                              fwk_id_t api_id,
                                              const void **api)
{
    if (fwk_id_is_equal(api_id, mod_juno_cdcel937_api_id_clock_driver))
        *api = &clock_driver_api;
    else if (fwk_id_is_equal(api_id, mod_juno_cdcel937_api_id_hdlcd_driver))
        *api = &hdlcd_driver_api;
    else
        return FWK_E_ACCESS;

    return FWK_SUCCESS;
}

static int juno_cdcel937_process_event(const struct fwk_event *event,
                                       struct fwk_event *resp_event)
{
    int status;
    struct juno_cdcel937_dev_ctx *ctx;
    struct mod_clock_driver_resp_params response_param;
    enum juno_cdcel937_output_type type;
    struct mod_i2c_event_param *param =
        (struct mod_i2c_event_param *)event->params;

    ctx = &ctx_table[fwk_id_get_element_idx(event->target_id)];

    switch (ctx->state) {
    case JUNO_CDCEL937_DEVICE_SET_RATE_SET_BLOCK_ACCESS_LENGTH:
        status = rate_set_block_access_length(ctx);
        if (status == FWK_PENDING) {
            ctx->state =
                JUNO_CDCEL937_DEVICE_SET_RATE_READ_PLL_CONFIG;
            return FWK_SUCCESS;
        }

        break;

    case JUNO_CDCEL937_DEVICE_SET_RATE_READ_PLL_CONFIG:
        /* check I2C request */
        if ((param->status != FWK_PENDING) &&
            (param->status != FWK_SUCCESS)) {
            status = param->status;
            break;
        }
        status = set_rate_read_pll_config(ctx);
        if (status == FWK_PENDING) {
            ctx->state =
                JUNO_CDCEL937_DEVICE_SET_RATE_WRITE_PLL_CONFIG;
            return FWK_SUCCESS;
        }

        break;

    case JUNO_CDCEL937_DEVICE_SET_RATE_WRITE_PLL_CONFIG:
        /* check I2C request */
        if ((param->status != FWK_PENDING) &&
            (param->status != FWK_SUCCESS)) {
            status = param->status;
            break;
        }
        status = set_rate_write_pll_config(ctx);
        if (status == FWK_PENDING) {
            ctx->state = JUNO_CDCEL937_DEVICE_SET_RATE_COMPLETE;
            return FWK_SUCCESS;
        }

        break;

    case JUNO_CDCEL937_DEVICE_SET_RATE_COMPLETE:
        /* check I2C request */
        if ((param->status != FWK_PENDING) &&
            (param->status != FWK_SUCCESS)) {
            status = param->status;
            break;
        }
        ctx->rate_hz = ctx->rate;
        ctx->rate_set = true;
        status = FWK_SUCCESS;
        break;

    case JUNO_CDCEL937_DEVICE_GET_RATE_SET_BLOCK_ACCESS_LENGTH:
        status = output_type_from_output_id(ctx->config->output_id, &type);
        if (status != FWK_SUCCESS)
            break;

        #if USE_OUTPUT_Y1
        if (type == JUNO_CDCEL937_OUTPUT_TYPE_Y1) {
            status = get_rate_y1_set_block_access_length(ctx);
            if (status == FWK_PENDING) {
                ctx->state =
                    JUNO_CDCEL937_DEVICE_GET_RATE_Y1_READ_Y1_CONFIG;
                return FWK_SUCCESS;
            }
        } else {
            status = rate_set_block_access_length(ctx);
            if (status == FWK_PENDING) {
                ctx->state =
                    JUNO_CDCEL937_DEVICE_GET_RATE_READ_PLL_CONFIG;
                return FWK_SUCCESS;
            }
        }

        #else
        status = rate_set_block_access_length(ctx);
        if (status == FWK_PENDING) {
            ctx->state =
                JUNO_CDCEL937_DEVICE_GET_RATE_READ_PLL_CONFIG;
            return FWK_SUCCESS;
        }

        #endif
        break;

    #if USE_OUTPUT_Y1
    case JUNO_CDCEL937_DEVICE_GET_RATE_Y1_READ_Y1_CONFIG:
        /* check I2C request */
        if ((param->status != FWK_PENDING) &&
        (param->status != FWK_SUCCESS)) {
            status = param->status;
            break;
        }
        status = get_rate_y1_read_y1_config(ctx);
        if (status == FWK_PENDING) {
            ctx->state =
                JUNO_CDCEL937_DEVICE_GET_RATE_Y1_CHECK_PDIV;
            return FWK_SUCCESS;
        }

        break;

    case JUNO_CDCEL937_DEVICE_GET_RATE_Y1_CHECK_PDIV:
        /* check I2C request */
        if ((param->status != FWK_PENDING) &&
           (param->status != FWK_SUCCESS)) {
            status = param->status;
            break;
        }
        status = get_rate_y1_check_pdiv(ctx);
        if (status == FWK_PENDING) {
            ctx->state =
                JUNO_CDCEL937_DEVICE_GET_RATE_Y1_READ_PLL_CONFIG;
            return FWK_SUCCESS;
        }

        break;

    case JUNO_CDCEL937_DEVICE_GET_RATE_Y1_READ_PLL_CONFIG:
        status = get_rate_y1_read_pll_config(ctx);
        if (status == FWK_PENDING) {
            ctx->state =
                JUNO_CDCEL937_DEVICE_GET_RATE_Y1_COMPLETE;
            return FWK_SUCCESS;
        }

        break;

    case JUNO_CDCEL937_DEVICE_GET_RATE_Y1_COMPLETE:
        status = get_rate_y1_calc(ctx);
        ctx->rate_hz = ctx->rate;
        ctx->rate_set = true;

        break;
    #endif

    case JUNO_CDCEL937_DEVICE_GET_RATE_READ_PLL_CONFIG:
        status = get_rate_read_pll_config(ctx);
        if (status == FWK_PENDING) {
            ctx->state =
                JUNO_CDCEL937_DEVICE_GET_RATE_COMPLETE;
            return FWK_SUCCESS;
        }

        break;

    case JUNO_CDCEL937_DEVICE_GET_RATE_COMPLETE:
        status = get_rate_calc(ctx);
        ctx->rate_hz = ctx->rate;
        ctx->rate_set = true;
        break;

    case JUNO_CDCEL937_DEVICE_IDLE:
    default:
            return FWK_E_PARAM;
    }

    ctx->state = JUNO_CDCEL937_DEVICE_IDLE;
    response_param.status = status;
    response_param.value.rate = ctx->rate_hz;
    ctx->driver_response_api->request_complete(ctx->config->clock_hal_id,
        &response_param);

    return FWK_SUCCESS;
}

const struct fwk_module module_juno_cdcel937 = {
    .name = "JUNO CDCEL937",
    .api_count = MOD_JUNO_CDCEL937_API_COUNT,
    .event_count = JUNO_CDCEL937_EVENT_IDX_COUNT,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = juno_cdcel937_init,
    .element_init = juno_cdcel937_dev_init,
    .bind = juno_cdcel937_bind,
    .process_bind_request = juno_cdcel937_process_bind_request,
    .process_event = juno_cdcel937_process_event,
};
