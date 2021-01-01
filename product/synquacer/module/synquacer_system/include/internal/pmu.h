/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_PMU_H
#define INTERNAL_PMU_H

#include <stdint.h>

/** interrupt factor bits */
#define PMU_INT_ST_PDIE (1U << 6)
#define PMU_INT_ST_PDNIE (1U << 5)
#define PMU_INT_ST_PONIE (1U << 4)
#define PMU_INT_ST_WUIE1 (1U << 1)
#define PMU_INT_ST_WUIE0 (1U << 0)

void pmu_on(uint32_t pd_on_flag);
void pmu_off(uint32_t pd_off_flag);
uint32_t pmu_read_pd_power_status(void);
void pmu_write_power_on_cycle(uint8_t pd_no, uint8_t value);
uint8_t pmu_read_power_on_cycle(uint8_t pd_no);
void pmu_enable_int(uint32_t enable_bit);
void pmu_disable_int(uint32_t disable_bit);
uint8_t pmu_read_int_satus(void);
void pmu_clr_int_satus(uint32_t clr_bit);
void pmu_on_wakeup(uint8_t pd_no);
void pmu_write_power_on_priority(uint8_t pd_no, uint8_t value);
void pmu_write_pwr_cyc_sel(uint32_t value);
uint32_t pmu_read_pwr_cyc_sel(void);

#endif /* INTERNAL_PMU_H */
