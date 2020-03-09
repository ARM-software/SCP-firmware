/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_CDCEL937_H
#define JUNO_CDCEL937_H

#include <mod_juno_cdcel937.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stdint.h>

#define JUNO_CDCEL937_I2C_TRANSFER_MODE_POS  7
#define JUNO_CDCEL937_I2C_BYTE_OFFSET_MASK   0x7F

#define JUNO_CDCEL937_TRANSFER_MODE_BLOCK \
    (0 << JUNO_CDCEL937_I2C_TRANSFER_MODE_POS)
#define JUNO_CDCEL937_TRANSFER_MODE_BYTE  \
    (1 << JUNO_CDCEL937_I2C_TRANSFER_MODE_POS)

#define JUNO_CDCEL937_PLL_MIN_FREQ_MHZ    80
#define JUNO_CDCEL937_PLL_MAX_FREQ_MHZ    230
#define JUNO_CDCEL937_OUTPUT_MIN_FREQ     (625 * FWK_KHZ)
#define JUNO_CDCEL937_OUTPUT_MAX_FREQ     (230 * FWK_MHZ)

#define JUNO_CDCEL937_BASE_CFG_REG        0x00
#define JUNO_CDCEL937_BASE_PLL1_REG       0x10
#define JUNO_CDCEL937_BASE_PLL2_REG       0x20
#define JUNO_CDCEL937_BASE_PLL3_REG       0x30

#define JUNO_CDCEL937_VCO_RANGE_LOWEST    0x0
#define JUNO_CDCEL937_VCO_RANGE_LOW       0x1
#define JUNO_CDCEL937_VCO_RANGE_HIGH      0x2
#define JUNO_CDCEL937_VCO_RANGE_HIGHEST   0x3

#define JUNO_CDCEL937_Y1_CLK_SOURCE_PLL   0x1

/* Bit field information for 8-bit register */
struct field {
    /* Mask of the bit field */
    uint8_t mask;

    /* Position of the bit field in the register */
    uint8_t pos;

    /* Offset of the register */
    unsigned int reg_idx;
};

/*
 * Create a bit field from its name and mask. The register offset and position
 * are deduced from them.
 */
#define FIELD(_name, _mask) \
    static const struct field _name = { \
        .reg_idx = _name##_IDX, \
        .mask = _mask, \
        .pos = __builtin_ctz(_mask), \
    }

/* Read and return the field of a 8-bit register array */
static inline uint8_t read_field(uint8_t *reg, struct field f)
{
    return (reg[f.reg_idx] & f.mask) >> f.pos;
}

/* Modify the field of a 8-bit register array */
static inline void write_field(uint8_t *reg, struct field f, uint8_t value)
{
    reg[f.reg_idx] &= ~f.mask;
    reg[f.reg_idx] |= ((value << f.pos) & f.mask);
}

enum cfg_reg0_idx {
    /* Register 0 */
    CFG_REG0_E_EL_IDX = 0,
    CFG_REG0_RID_IDX = 0,
    CFG_REG0_VID_IDX = 0,

    CFG_REG0_COUNT,
};

struct cfg_reg0 {
    uint8_t reg[CFG_REG0_COUNT];
};

/* Device identification */
FIELD(CFG_REG0_E_EL, 0x80);

/* Revision identification */
FIELD(CFG_REG0_RID, 0x70);

/* Vendor identification */
FIELD(CFG_REG0_VID, 0x0F);

enum cfg_reg6_idx {
    CFG_REG6_BCOUNT_IDX = 0,
    CFG_REG6_EEWRITE_IDX = 0,
    CFG_REG6_COUNT,
};

struct cfg_reg6 {
    uint8_t reg[CFG_REG6_COUNT];
};

/* Block read transfer size in bytes */
FIELD(CFG_REG6_BCOUNT, 0xFE);

/* Initiate EEPROM write cycle */
FIELD(CFG_REG6_EEWRITE, 0x01);

enum cfg_reg_y1_idx {
    /* Register 1 */
    CFG_REG_Y1_CLOCK_SOURCE_IDX = 1,   /* Y1 clock source selection */
    CFG_REG_Y1_SPICON_IDX = 1,         /* Mode select for p18-p19 */
    CFG_REG_Y1_OUTPUT_STATES_IDX = 1,  /* Y1_ST0 state definition */
    CFG_REG_Y1_PDIV_HIGH_IDX = 1,      /* Y1 output divider [9:8] */

    /* Register 2 */
    CFG_REG_Y1_PDIV_LOW_IDX = 2,       /* Y1 clock source selection */

    CFG_REG_Y1_COUNT,
};

struct cfg_reg_y1 {
    uint8_t reg[CFG_REG_Y1_COUNT];
};

FIELD(CFG_REG_Y1_CLOCK_SOURCE, 0x80);
FIELD(CFG_REG_Y1_SPICON, 0x40);
FIELD(CFG_REG_Y1_OUTPUT_STATES, 0x0C);
FIELD(CFG_REG_Y1_PDIV_HIGH, 0x03);
FIELD(CFG_REG_Y1_PDIV_LOW, 0xFF);

/*
 * PLL Configuration Register Structure
 * Note 1: The first four bytes of the register are not represented as they are
 *         not needed by the driver. We use bytes 0x4-0xB, inclusive.
 *
 * Note 2: The datasheet treats each PLL configuration register (1-3)
 *         individually but for memory efficiency we do not. Therefore we use X
 *         and Y in place of the numbers used to identify the outputs and
 *         dividers as there are two per PLL. Eg. Pdiv2/Pdiv3 -> PdivX/PdivY.
 *         M2/M3 -> MX/MY.
 *
 * Note 3: Register 0 is not used.
 *
 * Note 4: Each of the elements in the enum corresponds to a field indexed as
 *         per the cfg_reg_y1_idx enum.
 */
enum pll_cfg_reg_idx {
    /* Register 1 */
    PLL_CFG_REG_MUX_PLL_IDX = 1,        /* PLL mux */
    PLL_CFG_REG_MUX_X_IDX = 1,          /* Output X mux */
    PLL_CFG_REG_MUX_Y_IDX = 1,          /* Output Y mux */
    PLL_CFG_REG_RESERVED1_IDX = 1,      /* Reserved */
    PLL_CFG_REG_OUTPUT_STATES_IDX = 1,  /* Output X&Y state config */

    /* Register 2 */
    PLL_CFG_REG_RESERVED2_IDX = 2,      /* Reserved */

    /* Register 3 */
    PLL_CFG_REG_SCC_DC_IDX = 3,         /* SCC down/centre selection */
    PLL_CFG_REG_PDIV_X_IDX = 3,         /* PdivX output divider */

    /* Register 4 */
    PLL_CFG_REG_RESERVED3_IDX = 4,      /* Reserved */
    PLL_CFG_REG_PDIV_Y_IDX = 4,         /* PdivY output divider */

    /* Register 5 */
    PLL_CFG_REG_N_HIGH_IDX = 5,         /* PLL multiplier/divider: N[11:4] */

    /* Register 6 */
    PLL_CFG_REG_N_LOW_IDX = 6,          /* PLL multiplier/divider: N[3:0] */
    PLL_CFG_REG_R_HIGH_IDX = 6,         /* PLL multiplier/divider: R[8:5] */

    /* Register 7 */
    PLL_CFG_REG_R_LOW_IDX = 7,          /* PLL multiplier/divider: R[4:0] */
    PLL_CFG_REG_Q_HIGH_IDX = 7,         /* PLL multiplier/divider: Q[5:3] */

    /* Register 8 */
    PLL_CFG_REG_Q_LOW_IDX = 8,          /* PLL multiplier/divider: Q[2:0] */
    PLL_CFG_REG_P_IDX = 8,              /* PLL multiplier/divider: P */
    PLL_CFG_REG_VCO_RANGE_IDX = 8,      /* fVCO range selection */

    PLL_CFG_REG_COUNT,
};

struct pll_cfg_reg {
    uint8_t reg[PLL_CFG_REG_COUNT];
};

FIELD(PLL_CFG_REG_MUX_PLL, 0x80);
FIELD(PLL_CFG_REG_MUX_X, 0x40);
FIELD(PLL_CFG_REG_MUX_Y, 0x30);
FIELD(PLL_CFG_REG_OUTPUT_STATES, 0x03);
FIELD(PLL_CFG_REG_SCC_DC, 0x80);
FIELD(PLL_CFG_REG_PDIV_X, 0x7F);
FIELD(PLL_CFG_REG_PDIV_Y, 0x7F);
FIELD(PLL_CFG_REG_N_HIGH, 0xFF);
FIELD(PLL_CFG_REG_N_LOW, 0xF0);
FIELD(PLL_CFG_REG_R_HIGH, 0x0F);
FIELD(PLL_CFG_REG_R_LOW, 0xF8);
FIELD(PLL_CFG_REG_Q_HIGH, 0x07);
FIELD(PLL_CFG_REG_Q_LOW, 0xE0);
FIELD(PLL_CFG_REG_P, 0x1C);
FIELD(PLL_CFG_REG_VCO_RANGE, 0x03);

enum juno_cdcel937_output_type {
    JUNO_CDCEL937_OUTPUT_TYPE_Y1,
    JUNO_CDCEL937_OUTPUT_TYPE_LOW,
    JUNO_CDCEL937_OUTPUT_TYPE_HIGH,
    JUNO_CDCEL937_OUTPUT_TYPE_COUNT,
};

/*
 * states for i2c event sequencing
 */
enum juno_cdcel937_module_ctx_state {
    JUNO_CDCEL937_DEVICE_IDLE = 0,
    JUNO_CDCEL937_DEVICE_SET_RATE_SET_BLOCK_ACCESS_LENGTH,
    JUNO_CDCEL937_DEVICE_SET_RATE_READ_PLL_CONFIG,
    JUNO_CDCEL937_DEVICE_SET_RATE_WRITE_PLL_CONFIG,
    JUNO_CDCEL937_DEVICE_SET_RATE_COMPLETE,
    JUNO_CDCEL937_DEVICE_GET_RATE_SET_BLOCK_ACCESS_LENGTH,
    JUNO_CDCEL937_DEVICE_GET_RATE_READ_PLL_CONFIG,
    JUNO_CDCEL937_DEVICE_GET_RATE_COMPLETE,

    #if USE_OUTPUT_Y1
    JUNO_CDCEL937_DEVICE_GET_RATE_Y1_SET_BLOCK_ACCESS_LENGTH,
    JUNO_CDCEL937_DEVICE_GET_RATE_Y1_READ_Y1_CONFIG,
    JUNO_CDCEL937_DEVICE_GET_RATE_Y1_CHECK_PDIV,
    JUNO_CDCEL937_DEVICE_GET_RATE_Y1_READ_PLL_CONFIG,
    JUNO_CDCEL937_DEVICE_GET_RATE_Y1_COMPLETE,
    #endif
};

struct juno_cdcel937_dev_ctx {
    const struct mod_juno_cdcel937_dev_config *config;
    const struct mod_clock_driver_response_api *driver_response_api;
    uint64_t rate_hz;
    bool rate_set;
    struct pll_cfg_reg pll_config;
    struct cfg_reg_y1 y1_config;
    uint64_t rate;
    int index;
    enum juno_cdcel937_module_ctx_state state;
};

struct juno_cdcel937_module_ctx {
    const struct mod_juno_cdcel937_config *module_config;
    const struct mod_i2c_api *i2c_api;
    uint32_t cookie;
};

enum juno_cdcel937_event_idx {
    JUNO_CDCEL937_EVENT_IDX_SET_RATE,
    JUNO_CDCEL937_EVENT_IDX_GET_RATE,
    JUNO_CDCEL937_EVENT_IDX_COUNT,
};

static const fwk_id_t juno_cdcel937_event_id_set_rate =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_JUNO_CDCEL937,
        JUNO_CDCEL937_EVENT_IDX_SET_RATE);

static const fwk_id_t juno_cdcel937_event_id_get_rate =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_JUNO_CDCEL937,
        JUNO_CDCEL937_EVENT_IDX_GET_RATE);

#endif /* JUNO_CDCEL937_H */
