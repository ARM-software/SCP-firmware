/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_JUNO_RAM_H
#define MOD_JUNO_RAM_H

#include <fwk_id.h>

/*!
 * \ingroup GroupJunoModule
 * \defgroup GroupJunoRAM Juno RAM
 * \{
 */

/*!
 * \brief Module configuration.
 */
struct mod_juno_ram_config {
    /*!
     * \brief Identifier of the timer.
     *
     * \details Used for time-out when configuring the module's peripherals.
     */
    fwk_id_t timer_id;
};

/*!
 * \}
 */

#endif /* MOD_JUNO_RAM_H */
