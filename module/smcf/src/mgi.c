/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mgi.h"

#include <stddef.h>

/* Get the number of monitors supported by this MGI */
uint32_t mgi_get_num_of_monitors(struct smcf_mgi_reg *smcf_mgi)
{
    return ((smcf_mgi->GRP_ID & SMCF_GPR_ID_MON_NUM) >>
            SMCF_GPR_ID_MON_NUM_POS) +
        1;
}

/*
 * Enable monitor
 */
bool mgi_is_monitor_enabled(struct smcf_mgi_reg *smcf_mgi, uint32_t monitor)
{
    return ((smcf_mgi->MON_STAT & (1U << monitor)) != 0);
}

int mgi_enable_monitor(struct smcf_mgi_reg *smcf_mgi, uint32_t monitor)
{
    if (monitor >= mgi_get_num_of_monitors(smcf_mgi)) {
        return FWK_E_RANGE;
    }

    smcf_mgi->MON_REQ |= (1U << monitor);

    return FWK_SUCCESS;
}

int mgi_enable_monitor_blocking(struct smcf_mgi_reg *smcf_mgi, uint32_t monitor)
{
    int status;

    status = mgi_enable_monitor(smcf_mgi, monitor);
    if (status != FWK_SUCCESS) {
        return status;
    }

    while (!mgi_is_monitor_enabled(smcf_mgi, monitor)) {
        continue;
    }

    return FWK_SUCCESS;
}

int mgi_enable_all_monitor(struct smcf_mgi_reg *smcf_mgi)
{
    uint32_t i;
    int status;
    uint32_t num_of_monitor;

    num_of_monitor = mgi_get_num_of_monitors(smcf_mgi);

    for (i = 0; i < num_of_monitor; i++) {
        status = mgi_enable_monitor(smcf_mgi, i);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return FWK_SUCCESS;
}

/*
 * Disable monitor
 */
int mgi_disable_monitor(struct smcf_mgi_reg *smcf_mgi, uint32_t monitor)
{
    if (monitor >= mgi_get_num_of_monitors(smcf_mgi)) {
        return FWK_E_RANGE;
    }

    smcf_mgi->MON_REQ &= ~(1U << monitor);

    return FWK_SUCCESS;
}

int mgi_disable_monitor_blocking(
    struct smcf_mgi_reg *smcf_mgi,
    uint32_t monitor)
{
    int status;

    status = mgi_disable_monitor(smcf_mgi, monitor);
    if (status != FWK_SUCCESS) {
        return status;
    }

    while (mgi_is_monitor_enabled(smcf_mgi, monitor)) {
        continue;
    }

    return FWK_SUCCESS;
}

/*
 * Monitor modes
 */

/* Get the number (n) of MGI_MODE_REQ<n> and MGI_MODE_STAT<n> registers */
uint32_t mgi_get_number_of_mode_registers(struct smcf_mgi_reg *smcf_mgi)
{
    return ((smcf_mgi->FEAT1 & SMCF_MGI_FEAT1_MODE_REG) >>
            SMCF_MGI_FEAT1_MODE_REG_POS) +
        1;
}

/*
 * Get the number of bits in each MGI_MODE_REQ<n> and MGI_MODE_STAT<n> register
 */
uint32_t mgi_get_number_of_bits_in_mode_registers(struct smcf_mgi_reg *smcf_mgi)
{
    return ((smcf_mgi->FEAT1 & SMCF_MGI_FEAT1_MODE_LEN) >>
            SMCF_MGI_FEAT1_MODE_LEN_POS) +
        1;
}

/* Enable individual monitor mode programming using broadcast */
int mgi_enable_program_mode(struct smcf_mgi_reg *smcf_mgi, uint32_t monitor)
{
    if (monitor >= mgi_get_num_of_monitors(smcf_mgi)) {
        return FWK_E_RANGE;
    }

    smcf_mgi->MODE_BCAST |= (1U << monitor);

    return FWK_SUCCESS;
}

/* Enable multiple monitors mode programming using broadcast */
int mgi_enable_program_mode_multi(
    struct smcf_mgi_reg *smcf_mgi,
    uint32_t monitor_mask)
{
    uint32_t num_monitors;

    num_monitors = mgi_get_num_of_monitors(smcf_mgi);

    if (monitor_mask >= (1U << num_monitors)) {
        return FWK_E_RANGE;
    }

    smcf_mgi->MODE_BCAST |= monitor_mask;
    return FWK_SUCCESS;
}

/* Disable individual monitor mode programming using broadcast */
int mgi_disable_program_mode(struct smcf_mgi_reg *smcf_mgi, uint32_t monitor)
{
    if (monitor >= mgi_get_num_of_monitors(smcf_mgi)) {
        return FWK_E_RANGE;
    }

    smcf_mgi->MODE_BCAST &= ~(1U << monitor);

    return FWK_SUCCESS;
}

/* Set MODE for individual MODE_REQ register */
int mgi_set_monitor_mode(
    struct smcf_mgi_reg *smcf_mgi,
    uint32_t mode_idx,
    uint32_t value)
{
    uint32_t mode_registers_num = mgi_get_number_of_mode_registers(smcf_mgi);
    uint32_t mode_registers_bits =
        mgi_get_number_of_bits_in_mode_registers(smcf_mgi);
    uint32_t mode_mask = (1U << mode_registers_bits) - 1;

    if (mode_idx > (mode_registers_num - 1)) {
        return FWK_E_RANGE;
    }

    FWK_RW uint32_t *mode_req[SMCF_MGI_MAX_NUM_MODE_REG] = {
        &smcf_mgi->MODE_REQ0,
        &smcf_mgi->MODE_REQ1,
        &smcf_mgi->MODE_REQ2,
        &smcf_mgi->MODE_REQ3
    };

    *mode_req[mode_idx] = (value & mode_mask);

    return FWK_SUCCESS;
}

bool mgi_is_monitor_mode_updated(struct smcf_mgi_reg *smcf_mgi)
{
    uint32_t i;
    uint32_t mode_registers_num = mgi_get_number_of_mode_registers(smcf_mgi);

    FWK_RW uint32_t *mode_req[SMCF_MGI_MAX_NUM_MODE_REG] = {
        &smcf_mgi->MODE_REQ0,
        &smcf_mgi->MODE_REQ1,
        &smcf_mgi->MODE_REQ2,
        &smcf_mgi->MODE_REQ3
    };

    FWK_R uint32_t *mode_stat[SMCF_MGI_MAX_NUM_MODE_REG] = {
        &smcf_mgi->MODE_STAT0,
        &smcf_mgi->MODE_STAT1,
        &smcf_mgi->MODE_STAT2,
        &smcf_mgi->MODE_STAT3
    };

    for (i = 0; i < mode_registers_num; i++) {
        if (*mode_stat[i] != *mode_req[i]) {
            return false;
        }
    }

    return true;
}

/*
 * Monitor Sampling
 */

/* Functions for controlling sample enable register MGI_SMP_EN */
void mgi_enable_sample(struct smcf_mgi_reg *smcf_mgi)
{
    smcf_mgi->SMP_EN |= (1U << SMCF_MGI_SMP_EN_EN_POS);
}

void mgi_disable_sample(struct smcf_mgi_reg *smcf_mgi)
{
    smcf_mgi->SMP_EN &= ~(1U << SMCF_MGI_SMP_EN_EN_POS);
}

bool mgi_is_sample_ongoing(struct smcf_mgi_reg *smcf_mgi)
{
    return ((smcf_mgi->SMP_EN & SMCF_MGI_SMP_OG) >> SMCF_MGI_SMP_EN_OG_POS);
}

/* Set sample type in MGI_SMP_CFG */
int mgi_set_sample_type(struct smcf_mgi_reg *smcf_mgi, uint32_t type)
{
    /* Check if the requested type is supported by the hardware */
    if (type >= SMCF_MGI_SAMPLE_TYPE_COUNT) {
        return FWK_E_RANGE;
    }

    if ((type == SMCF_MGI_SAMPLE_TYPE_PERIODIC) &&
        !(smcf_mgi->FEAT0 & 1U << SMCF_MGI_FEAT0_PER_TIMER_POS)) {
        return FWK_E_SUPPORT;
    }

    if ((type == SMCF_MGI_SAMPLE_TYPE_TRIGGER_INPUT) &&
        !(smcf_mgi->FEAT0 & 1U << SMCF_MGI_FEAT0_TRIGGER_IN_POS)) {
        return FWK_E_SUPPORT;
    }

    smcf_mgi->SMP_CFG = (smcf_mgi->SMP_CFG & ~SMCF_MGI_SMP_CFG_SMP_TYP) |
        (type & SMCF_MGI_SMP_CFG_SMP_TYP);

    return FWK_SUCCESS;
}

/* Set sample period in MGI_SMP_PER */
int mgi_set_sample_period(struct smcf_mgi_reg *smcf_mgi, uint32_t period)
{
    if (!(smcf_mgi->FEAT0 & 1U << SMCF_MGI_FEAT0_PER_TIMER_POS)) {
        return FWK_E_SUPPORT;
    }

    smcf_mgi->SMP_PER = period;

    return FWK_SUCCESS;
}

/* Get the maximum sample delay supported */
uint32_t mgi_get_sample_delay_max(struct smcf_mgi_reg *smcf_mgi)
{
    uint32_t sample_delay_num_of_bits =
        ((smcf_mgi->FEAT1 & SMCF_MGI_FEAT1_SMP_DLY_LEN) >>
         SMCF_MGI_FEAT1_SMP_DLY_LEN_POS);

    return (1U << sample_delay_num_of_bits) - 1;
}

/* Set sample delay in MGI_SMP_DLY */
int mgi_set_sample_delay(struct smcf_mgi_reg *smcf_mgi, uint32_t delay)
{
    if (delay > mgi_get_sample_delay_max(smcf_mgi)) {
        return FWK_E_RANGE;
    }

    smcf_mgi->SMP_DLY = delay;

    return FWK_SUCCESS;
}

/* Get data info from MGI_DATA_INFO */
bool mgi_is_data_packed(struct smcf_mgi_reg *smcf_mgi)
{
    return (
        (smcf_mgi->DATA_INFO & SMCF_DATA_INFO_PACKED) >>
        SMCF_DATA_INFO_PACKED_POS);
}

uint32_t mgi_number_of_data_values_per_monitor(struct smcf_mgi_reg *smcf_mgi)
{
    return ((smcf_mgi->DATA_INFO & SMCF_DATA_INFO_DATA_PER_MON) >>
            SMCF_DATA_INFO_DATA_PER_MON_POS) +
        1;
}

uint32_t mgi_monitor_data_width(struct smcf_mgi_reg *smcf_mgi)
{
    return ((smcf_mgi->DATA_INFO & SMCF_DATA_INFO_MON_DATA_WIDTH) >>
            SMCF_DATA_INFO_MON_DATA_WIDTH_POS) +
        1;
}

bool scmf_is_set_data_address_supported(struct smcf_mgi_reg *smcf_mgi)
{
    return ((smcf_mgi->DATA_INFO & SMCF_DATA_INFO_ALT_ADDR) != 0);
}

/* DMA related functions */
bool mgi_is_dma_supported(struct smcf_mgi_reg *smcf_mgi)
{
    return (((smcf_mgi->FEAT0 >> SMCF_MGI_FEAT0_DMA_IF_POS) & 1) != 0);
}

int mgi_set_dma_data_address(struct smcf_mgi_reg *smcf_mgi, uint64_t address)
{
    if (!mgi_is_dma_supported(smcf_mgi)) {
        return FWK_E_SUPPORT;
    }

    if (address & 0x3) {
        /* The address is not 32-bit aligned as required by spec. */
        return FWK_E_ALIGN;
    }

    smcf_mgi->WADDR0 = (FWK_RW uint32_t)(address & UINT32_MAX);
    smcf_mgi->WADDR1 = (FWK_RW uint32_t)((address >> 32) & UINT32_MAX);

    return FWK_SUCCESS;
}

int mgi_enable_dma(struct smcf_mgi_reg *smcf_mgi)
{
    if (!mgi_is_dma_supported(smcf_mgi)) {
        return FWK_E_SUPPORT;
    }

    smcf_mgi->WREN |= (1U << SMCF_MGI_WREN_DMA_IF_POS);

    return FWK_SUCCESS;
}

/* Alternate data read address */
bool mgi_is_alternate_data_address_supported(struct smcf_mgi_reg *smcf_mgi)
{
    return (((smcf_mgi->DATA_INFO >> SMCF_DATA_INFO_ALT_ADDR_POS) & 1) != 0);
}

int mgi_set_alternate_data_address(
    struct smcf_mgi_reg *smcf_mgi,
    uint64_t address)
{
    if (!scmf_is_set_data_address_supported(smcf_mgi)) {
        return FWK_E_SUPPORT;
    }

    smcf_mgi->RADDR0 = (uint32_t)(address & UINT32_MAX);
    smcf_mgi->RADDR1 = (uint32_t)((address >> 32) & UINT32_MAX);

    return FWK_SUCCESS;
}

/* Get monitor error code */
uint32_t mgi_get_error_code(struct smcf_mgi_reg *smcf_mgi)
{
    return (
        (smcf_mgi->ERR_CODE & SMCF_MGI_ERR_CODE_ERROR_CODE) >>
        SMCF_MGI_ERR_CODE_ERROR_CODE_POS);
}

/* Get monitor id that generated an error */
uint32_t scmf_get_error_monitor_id(struct smcf_mgi_reg *smcf_mgi)
{
    return (
        (smcf_mgi->ERR_CODE & SMCF_MGI_ERR_CODE_MON_ID) >>
        SMCF_MGI_ERR_CODE_MON_ID_POS);
}

/*
 * If monitor id generated an error return the error code in err_code
 * and return true;
 */
bool mgi_is_monitor_id_generated_error(
    struct smcf_mgi_reg *smcf_mgi,
    const uint32_t monitor_id,
    uint32_t *err_code)
{
    if (((smcf_mgi->ERR_CODE & SMCF_MGI_ERR_CODE_MON_ID) >>
         SMCF_MGI_ERR_CODE_MON_ID_POS) == monitor_id) {
        *err_code = (smcf_mgi->ERR_CODE & SMCF_MGI_ERR_CODE_ERROR_CODE) >>
            SMCF_MGI_ERR_CODE_ERROR_CODE_POS;
        return true;
    }

    return false;
}

uint32_t mgi_get_start_sample_id(struct smcf_mgi_reg *smcf_mgi)
{
    if (((smcf_mgi->SMPID_START & SMCF_MGI_SMPID_STATUS) >>
         SMCF_MGI_SMPID_STATUS_POS) == 0) {
        return 0; /* Uninitialized / Invalid */
    }

    return (
        (smcf_mgi->SMPID_START & SMCF_MGI_SMPID_VALUE) >>
        SMCF_MGI_SMPID_VALUE_POS);
}

uint32_t mgi_get_end_sample_id(struct smcf_mgi_reg *smcf_mgi)
{
    if (((smcf_mgi->SMPID_END & SMCF_MGI_SMPID_STATUS) >>
         SMCF_MGI_SMPID_STATUS_POS) == 0) {
        return 0; /* Uninitialized / Invalid */
    }

    return (
        (smcf_mgi->SMPID_END & SMCF_MGI_SMPID_VALUE) >>
        SMCF_MGI_SMPID_VALUE_POS);
}

bool mgi_is_data_valid(struct smcf_mgi_reg *smcf_mgi, uint32_t monitor)
{
    return ((smcf_mgi->DVLD & (1U << monitor)) != 0);
}

/*
 * Interrupts
 */

/* Interrupt status */
bool mgi_is_the_source_triggered_the_interrupt(
    struct smcf_mgi_reg *smcf_mgi,
    uint32_t interrupt_source_pos)
{
    return (smcf_mgi->IRQ_STAT & (1 << interrupt_source_pos)) != 0;
}

/* Interrupt clear */
void mgi_interrupt_source_clear(
    struct smcf_mgi_reg *smcf_mgi,
    uint32_t interrupt_source_pos)
{
    smcf_mgi->IRQ_STAT &= ~(1 << interrupt_source_pos);
}

/* Interrupt mask */
void mgi_interrupt_source_mask(
    struct smcf_mgi_reg *smcf_mgi,
    uint32_t interrupt_source_pos)
{
    smcf_mgi->IRQ_MASK |= (1 << interrupt_source_pos);
}

/* Interrupt unmask */
void mgi_interrupt_source_unmask(
    struct smcf_mgi_reg *smcf_mgi,
    uint32_t interrupt_source_pos)
{
    smcf_mgi->IRQ_MASK &= ~(1 << interrupt_source_pos);
}

/* Tag */
bool mgi_is_tag_supported(struct smcf_mgi_reg *smcf_mgi)
{
    return (((smcf_mgi->FEAT0 >> SMCF_MGI_FEAT0_TAG_IN_POS) & 1) != 0);
}

uint32_t mgi_get_tag_length_in_bits(struct smcf_mgi_reg *smcf_mgi)
{
    return ((smcf_mgi->FEAT0 & SMCF_MGI_FEAT0_TAG_LEN) >>
            SMCF_MGI_FEAT0_TAG_LEN_POS) +
        1;
}

void mgi_enable_tag_id_write_to_ram(struct smcf_mgi_reg *smcf_mgi)
{
    smcf_mgi->WRCFG |= (1U << SMCF_MGI_WRCFG_TAG_ID_EN);
}

void mgi_enable_count_id_write_to_ram(struct smcf_mgi_reg *smcf_mgi)
{
    smcf_mgi->WRCFG |= (1U << SMCF_MGI_WRCFG_INCR_ID_EN);
}

void mgi_enable_valid_bits_write_to_ram(struct smcf_mgi_reg *smcf_mgi)
{
    smcf_mgi->WRCFG |= (1U << SMCF_MGI_WRCFG_DATA_VLD_EN);
}

void mgi_enable_group_id_write_to_ram(struct smcf_mgi_reg *smcf_mgi)
{
    smcf_mgi->WRCFG |= (1U << SMCF_MGI_WRCFG_GRP_ID_EN);
}

void mgi_request_start_id_wirte_to_ram(struct smcf_mgi_reg *smcf_mgi)
{
    smcf_mgi->WRCFG &= ~(SMCF_MGI_WRCFG_NUM_SAMPLE_ID);
    smcf_mgi->WRCFG |=
        (SMCF_MGI_WRCFG_WRITE_START_SAMPLE_ID
         << SMCF_MGI_WRCFG_NUM_SAMPLE_ID_POS);
}

void mgi_request_start_and_end_id_wirte_to_ram(struct smcf_mgi_reg *smcf_mgi)
{
    smcf_mgi->WRCFG &= ~(SMCF_MGI_WRCFG_NUM_SAMPLE_ID);
    smcf_mgi->WRCFG |=
        (SMCF_MGI_WRCFG_WRITE_START_AND_END_SAMPLE_ID
         << (SMCF_MGI_WRCFG_NUM_SAMPLE_ID_POS));
}
