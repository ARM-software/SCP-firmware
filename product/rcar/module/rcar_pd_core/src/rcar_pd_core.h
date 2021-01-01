/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RCAR_PD_CORE_H
#define RCAR_PD_CORE_H

/*!
 * \cond
 * @{
 */

#include <fwk_macros.h>

#include <stdbool.h>
#include <stdint.h>

#define CLUSTER_CPU_MASK 0x4
#define REQ_RESUME ((uint32_t)1U << 1U)
#define BIT_CA53_SCU ((uint32_t)1U << 21U)
#define BIT_CA57_SCU ((uint32_t)1U << 12U)
#define STATUS_PWRDOWN ((uint32_t)1U << 0U)
#define STATUS_PWRUP ((uint32_t)1U << 4U)

/*
 * Interface
 */
void SCU_power_up(uint32_t mpidr);
void rcar_pwrc_cpuoff(uint32_t mpidr);
void rcar_pwrc_cpuon(uint32_t mpidr);

/*!
 * \endcond
 * @}
 */

#endif /* RCAR_PD_CORE_H */
