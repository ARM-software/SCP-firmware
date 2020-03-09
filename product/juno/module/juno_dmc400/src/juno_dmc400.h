/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_DMC400_H
#define JUNO_DMC400_H

#include "system_clock.h"

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module_idx.h>

/* DDR frequency in MHz */
#define DDR_FREQUENCY_MHZ   UINT32_C(800)

/* Clock divider */
#define CLKDIV              ((SYSINCLK / (DDR_FREQUENCY_MHZ * FWK_MHZ)) - 1)

/* Time-out for DMC400 */
#define DMC400_CLOCK_DIV_SET_WAIT_TIMEOUT_US    1000
#define DMC400_CLOCK_SEL_SET_WAIT_TIMEOUT_US    1000
#define DMC400_CLOCK_ENABLE_WAIT_TIMEOUT_US     1000
#define DMC400_PHY_INIT_WAIT_TIMEOUT_US         (100 * 1000)
#define DMC400_CONFIG_WAIT_TIMEOUT_US           (100 * 1000)
#define DMC400_TRAINING_TIMEOUT_US              (1000 * 1000)

/*
 * Event indices
 */
enum juno_dmc400_event_idx {
    /* Training event */
    JUNO_DMC400_EVENT_IDX_TRAINING,

    /* Number of defined events */
    JUNO_DMC400_EVENT_IDX_COUNT
};

/*
 * Training event identifier
 */
static const fwk_id_t juno_dmc400_event_id_training =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_JUNO_DMC400,
                      JUNO_DMC400_EVENT_IDX_TRAINING);

#endif  /* JUNO_DMC400_H */
