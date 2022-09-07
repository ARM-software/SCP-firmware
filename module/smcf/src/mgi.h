/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MGI_H
#define MGI_H

/*!
 * \cond
 */

#include <fwk_attributes.h>
#include <fwk_macros.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>

/*
 * SMCF register definitions
 */

#define MGI_GRP_ID_OFFSET      0x00
#define MGI_DATA_INFO_OFFSET   0x08
#define MGI_FEAT0_OFFSET       0x010
#define MGI_FEAT1_OFFSET       0x018
#define MGI_SMP_EN_OFFSET      0x030
#define MGI_SMP_CFG_OFFSET     0x038
#define MGI_SMP_PER_OFFSET     0x040
#define MGI_SMP_DLY_OFFSET     0x048
#define MGI_MON_REQ_OFFSET     0x060
#define MGI_MON_STAT_OFFSET    0x070
#define MGI_MODE_BCAST_OFFSET  0x080
#define MGI_MODE_REQ0_OFFSET   0x090
#define MGI_MODE_REQ1_OFFSET   0x098
#define MGI_MODE_REQ2_OFFSET   0x00A0
#define MGI_MODE_REQ3_OFFSET   0x00A8
#define MGI_MODE_STAT0_OFFSET  0x00C0
#define MGI_MODE_STAT1_OFFSET  0x00C8
#define MGI_MODE_STAT2_OFFSET  0x00D0
#define MGI_MODE_STAT3_OFFSET  0x00D8
#define MGI_IRQ_STAT_OFFSET    0x0100
#define MGI_IRQ_MASK_OFFSET    0x0110
#define MGI_TRG_MASK_OFFSET    0x0140
#define MGI_ERR_CODE_OFFSET    0x0150
#define MGI_WREN_OFFSET        0x0160
#define MGI_WRCFG_OFFSET       0x0168
#define MGI_WADDR0_OFFSET      0x0170
#define MGI_WADDR1_OFFSET      0x0174
#define MGI_RADDR0_OFFSET      0x0180
#define MGI_RADDR1_OFFSET      0x0184
#define MGI_DISCON_ID_OFFSET   0x0190
#define MGI_CON_STAT_OFFSET    0x0198
#define MGI_CMD_SEND0_OFFSET   0x01B0
#define MGI_CMD_SEND1_OFFSET   0x01B8
#define MGI_CMD_RECV0_OFFSET   0x01C0
#define MGI_CMD_RECV1_OFFSET   0x01C8
#define MGI_ATYP0_OFFSET       0x0200
#define MGI_AVAL_LOW0_OFFSET   0x0208
#define MGI_AVAL_HIGH0_OFFSET  0x020C
#define MGI_ATYP1_OFFSET       0x0210
#define MGI_AVAL_LOW1_OFFSET   0x0218
#define MGI_AVAL_HIGH1_OFFSET  0x021C
#define MGI_ATYP2_OFFSET       0x0220
#define MGI_AVAL_LOW2_OFFSET   0x0228
#define MGI_AVAL_HIGH2_OFFSET  0x022C
#define MGI_ATYP3_OFFSET       0x0230
#define MGI_AVAL_LOW3_OFFSET   0x0238
#define MGI_AVAL_HIGH3_OFFSET  0x023C
#define MGI_ATYP4_OFFSET       0x0240
#define MGI_AVAL_LOW4_OFFSET   0x0248
#define MGI_AVAL_HIGH4_OFFSET  0x024C
#define MGI_ATYP5_OFFSET       0x0250
#define MGI_AVAL_LOW5_OFFSET   0x0258
#define MGI_AVAL_HIGH5_OFFSET  0x025C
#define MGI_ATYP6_OFFSET       0x0260
#define MGI_AVAL_LOW6_OFFSET   0x0268
#define MGI_AVAL_HIGH6_OFFSET  0x026C
#define MGI_DATA_OFFSET        0X0700
#define MGI_DVLD_OFFSET        0xF00
#define MGI_TAG0_OFFSET        0xF10
#define MGI_TAG1_OFFSET        0xF14
#define MGI_TAG2_OFFSET        0xF18
#define MGI_TAG3_OFFSET        0xF1C
#define MGI_SMPID_START_OFFSET 0xF40
#define MGI_SMPID_END_OFFSET   0xF48
#define MGI_IIDR_OFFSET        0xFC0
#define MGI_AIDR_OFFSET        0xFC8

#define MGI_DATA_NUM_VALUES 512

struct smcf_mgi_reg {
    FWK_R uint32_t GRP_ID;
    uint32_t RESERVED0;
    FWK_R uint32_t DATA_INFO;
    uint32_t RESERVED1;
    FWK_R uint32_t FEAT0;
    uint32_t RESERVED2;
    FWK_R uint32_t FEAT1;
    uint8_t RESERVED3[0x030 - 0x018 - 4];
    FWK_RW uint32_t SMP_EN;
    uint32_t RESERVED4;
    FWK_RW uint32_t SMP_CFG;
    uint32_t RESERVED5;
    FWK_RW uint32_t SMP_PER;
    uint32_t RESERVED6;
    FWK_RW uint32_t SMP_DLY;
    uint8_t RESERVED7[0x060 - 0x048 - 4];
    FWK_RW uint32_t MON_REQ;
    uint8_t RESERVED8[0x070 - 0x060 - 4];
    FWK_R uint32_t MON_STAT;
    uint8_t RESERVED9[0x080 - 0x070 - 4];
    FWK_RW uint32_t MODE_BCAST;
    uint8_t RESERVED10[0x090 - 0x080 - 4];
    FWK_RW uint32_t MODE_REQ0;
    uint32_t RESERVED11;
    FWK_RW uint32_t MODE_REQ1;
    uint32_t RESERVED12;
    FWK_RW uint32_t MODE_REQ2;
    uint32_t RESERVED13;
    FWK_RW uint32_t MODE_REQ3;
    uint8_t RESERVED14[0x0C0 - 0x090 - 28];
    FWK_R uint32_t MODE_STAT0;
    uint32_t RESERVED15;
    FWK_R uint32_t MODE_STAT1;
    uint32_t RESERVED16;
    FWK_R uint32_t MODE_STAT2;
    uint32_t RESERVED17;
    FWK_R uint32_t MODE_STAT3;
    uint8_t RESERVED18[0x0100 - 0x00D8 - 4];
    FWK_RW uint32_t IRQ_STAT;
    uint8_t RESERVED19[0x0110 - 0x0100 - 4];
    FWK_RW uint32_t IRQ_MASK;
    uint8_t RESERVED20[0x0140 - 0x0110 - 4];
    FWK_RW uint32_t TRG_MASK;
    uint8_t RESERVED21[MGI_ERR_CODE_OFFSET - MGI_TRG_MASK_OFFSET - 4];
    FWK_R uint32_t ERR_CODE;
    uint8_t RESERVED22[MGI_WREN_OFFSET - MGI_ERR_CODE_OFFSET - 4];
    FWK_RW uint32_t WREN;
    uint32_t RESERVED23;
    FWK_RW uint32_t WRCFG;
    uint32_t RESERVED24;
    FWK_RW uint32_t WADDR0;
    FWK_RW uint32_t WADDR1;
    uint8_t RESERVED25[MGI_RADDR0_OFFSET - MGI_WADDR1_OFFSET - 4];
    FWK_RW uint32_t RADDR0;
    FWK_RW uint32_t RADDR1;
    uint8_t RESERVED26[MGI_DISCON_ID_OFFSET - MGI_RADDR1_OFFSET - 4];
    FWK_R uint32_t DISCON_ID;
    uint32_t RESERVED27;
    FWK_R uint32_t CON_STAT;
    uint8_t RESERVED28[MGI_CMD_SEND0_OFFSET - MGI_CON_STAT_OFFSET - 4];
    FWK_RW uint32_t CMD_SEND0;
    uint32_t RESERVED29;
    FWK_RW uint32_t CMD_SEND1;
    uint32_t RESERVED30;
    FWK_R uint32_t CMD_RECV0;
    uint32_t RESERVED31;
    FWK_R uint32_t CMD_RECV1;
    uint8_t RESERVED32[MGI_ATYP0_OFFSET - MGI_CMD_RECV1_OFFSET - 4];
    FWK_RW uint32_t ATYP0;
    uint32_t RESERVED33;
    FWK_RW uint32_t AVAL_LOW0;
    FWK_RW uint32_t AVAL_HIGH0;
    FWK_RW uint32_t ATYP1;
    uint32_t RESERVED34;
    FWK_RW uint32_t AVAL_LOW1;
    FWK_RW uint32_t AVAL_HIGH1;
    FWK_RW uint32_t ATYP2;
    uint32_t RESERVED35;
    FWK_RW uint32_t AVAL_LOW2;
    FWK_RW uint32_t AVAL_HIGH2;
    FWK_RW uint32_t ATYP3;
    uint32_t RESERVED36;
    FWK_RW uint32_t AVAL_LOW3;
    FWK_RW uint32_t AVAL_HIGH3;
    FWK_RW uint32_t ATYP4;
    uint32_t RESERVED37;
    FWK_RW uint32_t AVAL_LOW4;
    FWK_RW uint32_t AVAL_HIGH4;
    FWK_RW uint32_t ATYP5;
    uint32_t RESERVED38;
    FWK_RW uint32_t AVAL_LOW5;
    FWK_RW uint32_t AVAL_HIGH5;
    FWK_RW uint32_t ATYP6;
    uint32_t RESERVED39;
    FWK_RW uint32_t AVAL_LOW6;
    FWK_RW uint32_t AVAL_HIGH6;
    uint8_t RESERVED40[MGI_DATA_OFFSET - MGI_AVAL_HIGH6_OFFSET - 4];
    FWK_R uint32_t DATA[MGI_DATA_NUM_VALUES];
    FWK_R uint32_t DVLD;
    uint8_t RESERVED41[MGI_TAG0_OFFSET - MGI_DVLD_OFFSET - 4];
    FWK_R uint32_t TAG0;
    FWK_R uint32_t TAG1;
    FWK_R uint32_t TAG2;
    FWK_R uint32_t TAG3;
    uint8_t RESERVED42[MGI_SMPID_START_OFFSET - MGI_TAG3_OFFSET - 4];
    FWK_R uint32_t SMPID_START;
    uint32_t RESERVED43;
    FWK_R uint32_t SMPID_END;
    uint8_t RESERVED44[MGI_IIDR_OFFSET - MGI_SMPID_END_OFFSET - 4];
    FWK_R uint32_t IIDR;
    uint32_t RESERVED46;
    FWK_R uint32_t AIDR;
};

/*
 * Bit definitions for MGI_GRP_ID
 */
#define SMCF_GRP_ID_GRP_ID_POS  0
#define SMCF_GPR_ID_MON_NUM_POS 16

#define SMCF_GPR_ID_GRP_ID  UINT32_C(0X000007FF)
#define SMCF_GPR_ID_MON_NUM UINT32_C(0X001F0000)

/*
 * Bit definitions for MGI_DATA_INFO
 */
#define SMCF_DATA_INFO_DATA_PER_MON_POS   0
#define SMCF_DATA_INFO_PACKED_POS         20
#define SMCF_DATA_INFO_ALT_ADDR_POS       21
#define SMCF_DATA_INFO_MON_DATA_WIDTH_POS 22

#define SMCF_DATA_INFO_DATA_PER_MON   UINT32_C(0X0000FFFF)
#define SMCF_DATA_INFO_PACKED         UINT32_C(0X00100000)
#define SMCF_DATA_INFO_ALT_ADDR       UINT32_C(0X00200000)
#define SMCF_DATA_INFO_MON_DATA_WIDTH UINT32_C(0X0FC00000)

/*
 * Bit definitions for MGI_FEAT0 Feature Identification Register 0
 */
/* FEAT0.ALERT_NUM = Indicates the number of supported alerts. */
#define SMCF_MGI_FEAT0_ALERT_NUM_POS 0
/* FEAT0.TAG_LEN = Indicates the bit width of the tag value if present. */
#define SMCF_MGI_FEAT0_TAG_LEN_POS         8
#define SMCF_MGI_FEAT0_SINGLE_MON_MODE_POS 23
#define SMCF_MGI_FEAT0_ALT_DELTA_POS       24
/* FEAT0.TAG_IN = Indicates if the tag input is present. */
#define SMCF_MGI_FEAT0_TAG_IN_POS 25
/* FEAT0.TRIG_IN = Indicates if the input trigger is supported. */
#define SMCF_MGI_FEAT0_TRIGGER_IN_POS  26
#define SMCF_MGI_FEAT0_TRIGGER_OUT_POS 27
/* FEAT0.DMA_IF = Indicates if the DMA interface is present. */
#define SMCF_MGI_FEAT0_DMA_IF_POS       28
#define SMCF_MGI_FEAT0_USER_DEF_CMD_POS 29
#define SMCF_MGI_FEAT0_MON_DISCON_POS   30
/* FEAT0.PER_TIMER = Indicates if the periodic timer is present.*/
#define SMCF_MGI_FEAT0_PER_TIMER_POS 31

#define SMCF_MGI_FEAT0_ALERT_NUM UINT32_C(0X00000007)
#define SMCF_MGI_FEAT0_TAG_LEN   UINT32_C(0X00007F00)

/*
 * Bit definitions for MGI_FEAT1 Feature Identification Register 1
 * FEAT1.MODE_REG = Number of monitor mode request and status register pairs
 *                  This can contain values between 0 and 4 only
 * FEAT1.MODE_LEN = Width of each monitor mode request and status registers
 * FEAT1.SMP_DLY_LEN = The number of bits in the Sample Delay register
 */
#define SMCF_MGI_FEAT1_MODE_REG_POS    0
#define SMCF_MGI_FEAT1_MODE_LEN_POS    8
#define SMCF_MGI_FEAT1_SMP_DLY_LEN_POS 16

#define SMCF_MGI_FEAT1_MODE_REG    UINT32_C(0X00000007)
#define SMCF_MGI_FEAT1_MODE_LEN    UINT32_C(0X00001F00)
#define SMCF_MGI_FEAT1_SMP_DLY_LEN UINT32_C(0X003F0000)

#define SMCF_MGI_MAX_NUM_MODE_REG 4

/*
 * Bit definitions for MGI_SMP_EN
 */
#define SMCF_MGI_SMP_EN_EN_POS 0
#define SMCF_MGI_SMP_EN_OG_POS 1

#define SMCF_MGI_SMP_EN UINT32_C(0X00000001)
#define SMCF_MGI_SMP_OG UINT32_C(0X00000002)

/*
 * Bit definitions for MGI_SMP_CFG
 */
#define SMCF_MGI_SMP_CFG_SMP_TYP_POS 0

#define SMCF_MGI_SMP_CFG_SMP_TYP UINT32_C(0X00000003)
enum smcf_mgi_sample_type {
    SMCF_MGI_SAMPLE_TYPE_MANUAL,
    SMCF_MGI_SAMPLE_TYPE_PERIODIC,
    SMCF_MGI_SAMPLE_TYPE_DATA_READ,
    SMCF_MGI_SAMPLE_TYPE_TRIGGER_INPUT,
    SMCF_MGI_SAMPLE_TYPE_COUNT
};

/*
 * Bit definitions for MGI_SMPID both START and END
 */
#define SMCF_MGI_SMPID_VALUE_POS  0
#define SMCF_MGI_SMPID_STATUS_POS 28

#define SMCF_MGI_SMPID_VALUE  UINT32_C(0X0FFFFFFF)
#define SMCF_MGI_SMPID_STATUS UINT32_C(0XF0000000)

/* Sample Identifier Encoding */
enum smcf_sample_identifier_status_encoding {
    SMCF_SAMPLE_ID_UNINITIALIZED_INVALID = 0x0,
    SMCF_SAMPLE_ID_POWERED_OFF = 0x1,
    SMCF_SAMPLE_ID_VALID_COUNT = 0x8,
    SMCF_SAMPLE_ID_VALID_TAG_LEN = 0x9,
};

/*
 * Bit definitions for MGI_ERR_CODE Error Code Register
 */
#define SMCF_MGI_ERR_CODE_ERROR_CODE_POS 0
#define SMCF_MGI_ERR_CODE_MON_ID_POS     8

#define SMCF_MGI_ERR_CODE_ERROR_CODE UINT32_C(0X0000001F)
#define SMCF_MGI_ERR_CODE_MON_ID     UINT32_C(0X00001F00)

enum smcf_mgi_error_code {
    SMCF_MGI_ERR_CODE_SAMPLE = 0,
    SMCF_MGI_ERR_CODE_SAMPLE_ON_DISABLED_MONITOR,
    SMCF_MGI_ERR_CODE_ENABLE,
    SMCF_MGI_ERR_CODE_DISABLE,
    SMCF_MGI_ERR_CODE_MODE,
    SMCF_MGI_ERR_CODE_MONITOR_MODE_ON_DISABLED_MONITOR,
    SMCF_MGI_ERR_CODE_UNKNOWN_CMD_MLI = 7,
    SMCF_MGI_ERR_CODE_SAMPLE_PERIOD_WARNING = 16,
    SMCF_MGI_ERR_CODE_UNKNOWN_CMD_MGI = 23,
};

/*
 * Bit definitions for MGI_WREN, Data Write Enable Register
 */
/* MGI_WREN.DMA_IF = Data Write Enable, as configured in MGI_WRCFG Register. */
#define SMCF_MGI_WREN_DMA_IF_POS 0

/*
 * Bit definitions for MGI_IRQ_STAT and MGI_IRQ_MASK.
 * Interrupt Status and Interrupt Mask Registers.
 * These two registers are used to find and mask the interrupt event source.
 */
enum smcf_mgi_interrupt_event_source {
    /* Monitor Sample Data Set Complete Interrupt Event. */
    SMCF_MGI_IRQ_SOURCE_SMP_CMP = 0,
    /* Monitor Enable Request Complete Interrupt Event. */
    SMCF_MGI_IRQ_SOURCE_MON_EN,
    /* Monitor Mode Request Complete Interrupt Event. */
    SMCF_MGI_IRQ_SOURCE_MON_MODE,
    /* User Defined Command Received Interrupt Event. */
    SMCF_MGI_IRQ_SOURCE_CMD_RECV,
    /* Error Interrupt Event. */
    SMCF_MGI_IRQ_SOURCE_ERR,
    /*Monitor Trigger Interrupt Event. */
    SMCF_MGI_IRQ_SOURCE_MON_TRIG,
    /* Input Trigger Interrupt Event. */
    SMCF_MGI_IRQ_SOURCE_IN_TRIG,
    /* Configuration Request Interrupt Event. */
    SMCF_MGI_IRQ_SOURCE_CFG,
    /* Data Write Complete Interrupt Event. */
    SMCF_MGI_IRQ_SOURCE_DATA_WR,

    /* Alert Interrupt Event. */
    SMCF_MGI_IRQ_SOURCE_ALT0 = 16,
    SMCF_MGI_IRQ_SOURCE_ALT1,
    SMCF_MGI_IRQ_SOURCE_ALT2,
    SMCF_MGI_IRQ_SOURCE_ALT3,
    SMCF_MGI_IRQ_SOURCE_ALT4,
    SMCF_MGI_IRQ_SOURCE_ALT5,
    SMCF_MGI_IRQ_SOURCE_ALT6,
    SMCF_MGI_IRQ_SOURCE_MAX
};

/*
 * Bit definitions for MGI_WRCFG, Data Write Configuration Register
 */
/* Write Condition */
#define SMCF_MGI_WRCFG_WR_COND_POS 0
/* Number of Sample Identifiers written */
#define SMCF_MGI_WRCFG_NUM_SAMPLE_ID_POS 4
/* Group ID Enable */
#define SMCF_MGI_WRCFG_GRP_ID_EN 8
/* Data Valid Enable */
#define SMCF_MGI_WRCFG_DATA_VLD_EN 9
/* Incrementing Count Sample ID Enable */
#define SMCF_MGI_WRCFG_INCR_ID_EN 10
/* Tag Sample ID Enable */
#define SMCF_MGI_WRCFG_TAG_ID_EN 11
/* Synchronize the Alert Interrupt with the DMA Write Complete Interrupt */
#define SMCF_MGI_WRCFG_IRQ_WR_SYNC 16

/* SMCF_MGI_WRCFG.WR_COND */
#define SMCF_MGI_WRCFG_WR_COND UINT32_C(0X00000003)

enum smcf_mgi_dma_data_write_condition {
    /* Data is written when every monitor sample data set is complete */
    SMCF_MGI_WRCFG_DATA_WRITE_COMPLETE,
    /* Data is written once when an alert is triggered */
    SMCF_MGI_WRCFG_DATA_WRITE_ONCE_ALERT,
    /* Data writing starts when an alert is triggered and continues until data
       writing is disabled */
    SMCF_MGI_WRCFG_DATA_WRITE_START_ALERT_UNTIL_DISABLED,
};

/* SMCF_MGI_WRCFG.WR_COND */
#define SMCF_MGI_WRCFG_NUM_SAMPLE_ID UINT32_C(0X00000030)

enum smcf_mgi_wrcfg_number_of_sample_id {
    SMCF_MGI_WRCFG_DISABLE_SAMPLE_ID,
    SMCF_MGI_WRCFG_WRITE_START_SAMPLE_ID,
    SMCF_MGI_WRCFG_WRITE_START_AND_END_SAMPLE_ID,
};

/* Get SMCF number of monitors */
uint32_t mgi_get_num_of_monitors(struct smcf_mgi_reg *smcf_mgi);

/*
 * Enable individual monitor based on the monitor index.
 * This operation might not be applied by the time this function returns.
 * The status register must be checked using `smcf_is_monitor_enabled`
 * before assuming the new status has been applied.
 */
int mgi_enable_monitor(struct smcf_mgi_reg *smcf_mgi, uint32_t monitor);

/* Enable all monitor */
int mgi_enable_all_monitor(struct smcf_mgi_reg *smcf_mgi);

/* Disable individual monitor based on the monitor index */
int mgi_disable_monitor(struct smcf_mgi_reg *smcf_mgi, uint32_t monitor);

/* Check monitor is enabled */
bool mgi_is_monitor_enabled(struct smcf_mgi_reg *smcf_mgi, uint32_t monitor);

/* Get the number of MGI_MODE registers */
uint32_t mgi_get_number_of_mode_registers(struct smcf_mgi_reg *smcf_mgi);

/* Get the number of bits in each MGI_MODE register */
uint32_t mgi_get_number_of_bits_in_mode_registers(
    struct smcf_mgi_reg *smcf_mgi);

/* Enable program monitor mode */
int mgi_enable_program_mode(struct smcf_mgi_reg *smcf_mgi, uint32_t monitor);

/* Disable program monitor mode */
int mgi_disable_program_mode(struct smcf_mgi_reg *smcf_mgi, uint32_t monitor);

/* Enable multiple monitors mode programming using broadcast */
int mgi_enable__program_mode_multi(
    struct smcf_mgi_reg *smcf_mgi,
    uint32_t monitor_mask);

/* Set MODE for individual MODE_REQ reigester */
int mgi_set_monitor_mode(
    struct smcf_mgi_reg *smcf_mgi,
    uint32_t mode_idx,
    uint32_t value);

/* Enable monitor. Will not return until the request is applied */
int mgi_enable_monitor_blocking(
    struct smcf_mgi_reg *smcf_mgi,
    uint32_t monitor);

/* Disable mointor. Will not return until the reqest is applied */
int mgi_disable_monitor_blocking(
    struct smcf_mgi_reg *smcf_mgi,
    uint32_t monitor);

/* Set sample type */
int mgi_set_sample_type(struct smcf_mgi_reg *smcf_mgi, uint32_t type);

/* Enable sample */
void mgi_enable_sample(struct smcf_mgi_reg *smcf_mgi);

/* Is sampling ongoing */
bool mgi_is_sample_ongoing(struct smcf_mgi_reg *smcf_mgi);

/* Set sample delay value */
int mgi_set_sample_delay(struct smcf_mgi_reg *smcf_mgi, uint32_t delay);

/* This indicates multiple monitor data values are placed in a single 32-bit */
bool mgi_is_data_packed(struct smcf_mgi_reg *smcf_mgi);

/* The number of data values generated per monitor for each sample */
uint32_t mgi_number_of_data_values_per_monitor(struct smcf_mgi_reg *smcf_mgi);

/* The bit width of the data values generated by each monitor */
uint32_t mgi_monitor_data_width(struct smcf_mgi_reg *smcf_mgi);

/* is alternate address supported */
bool mgi_is_alternate_data_address_supported(struct smcf_mgi_reg *smcf_mgi);

/* Is DMA supported */
bool mgi_is_dma_supported(struct smcf_mgi_reg *smcf_mgi);

/* Set DMA data address */
int mgi_set_dma_data_address(struct smcf_mgi_reg *smcf_mgi, uint64_t address);

/* Enable DMA */
int mgi_enable_dma(struct smcf_mgi_reg *smcf_mgi);

/* Unpack data */
uint32_t mgi_unpack_data(
    uint32_t volatile const *data_addr,
    uint32_t mli_idx,
    uint32_t data_idx,
    uint32_t num_of_data,
    uint32_t alignment);

/* Get start sample ID */
uint32_t mgi_get_start_sample_id(struct smcf_mgi_reg *smcf_mgi);

/* Get end sample ID */
uint32_t mgi_get_end_sample_id(struct smcf_mgi_reg *smcf_mgi);

/* Check data valid */
bool mgi_is_data_valid(struct smcf_mgi_reg *smcf_mgi, uint32_t monitor);

/*
 * Interrupts
 */
bool mgi_is_the_source_triggered_the_interrupt(
    struct smcf_mgi_reg *smcf_mgi,
    uint32_t interrupt_source_pos);
void mgi_interrupt_source_clear(
    struct smcf_mgi_reg *smcf_mgi,
    uint32_t interrupt_source_pos);
void mgi_interrupt_source_mask(
    struct smcf_mgi_reg *smcf_mgi,
    uint32_t interrupt_source_pos);
void mgi_interrupt_source_unmask(
    struct smcf_mgi_reg *smcf_mgi,
    uint32_t interrupt_source_pos);

/* Tag */
bool mgi_is_tag_supported(struct smcf_mgi_reg *smcf_mgi);
uint32_t mgi_get_tag_length_in_bits(struct smcf_mgi_reg *smcf_mgi);

/* Sample ID */
void mgi_enable_tag_id_write_to_ram(struct smcf_mgi_reg *smcf_mgi);
void mgi_enable_count_id_write_to_ram(struct smcf_mgi_reg *smcf_mgi);
void mgi_enable_valid_bits_write_to_ram(struct smcf_mgi_reg *smcf_mgi);
void mgi_enable_group_id_write_to_ram(struct smcf_mgi_reg *smcf_mgi);
void mgi_request_start_id_wirte_to_ram(struct smcf_mgi_reg *smcf_mgi);
void mgi_request_start_and_end_id_wirte_to_ram(struct smcf_mgi_reg *smcf_mgi);

/*!
 * \endcond
 */

#endif /* MGI_H */
