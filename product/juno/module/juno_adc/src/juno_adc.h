/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_ADC_H
#define JUNO_ADC_H

/* Coefficients for ADC values */
#define ADC_VOLT_CONST      1622
#define ADC_CURRENT_CONST1  381
#define ADC_CURRENT_CONST2  761
#define ADC_POWER_CONST1    617402
#define ADC_POWER_CONST2    1234803
#define ADC_ENERGY_CONST1   617402
#define ADC_ENERGY_CONST2   1234803

/* Field masks for the ADC values */
#define JUNO_ADC_SYS_REG_AMPS_MASK      UINT32_C(0x00000FFF)
#define JUNO_ADC_SYS_REG_VOLT_MASK      UINT32_C(0x00000FFF)
#define JUNO_ADC_SYS_REG_POWER_MASK     UINT32_C(0x00FFFFFF)

/* Multiplying factors */
#define JUNO_ADC_AMPS_MULTIPLIER 1000
#define JUNO_ADC_VOLT_MULTIPLIER 1000
#define JUNO_ADC_WATTS_MULTIPLIER (1000 * 1000)
#define JUNO_ADC_JOULE_MULTIPLIER 100

#endif /* JUNO_ADC_H */
