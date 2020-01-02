/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Standard banner templates for SCP and MCP firmware.
 */

#ifndef FWK_BANNER_H
#define FWK_BANNER_H

/*!
 * \addtogroup GroupLibFramework Framework
 * @{
 */

/*!
 * \defgroup GroupBanner Banner Templates
 * @{
 */

/*!
 * \brief Standard SCP banner string.
 *
 * \hideinitializer
 */
#define FWK_BANNER_SCP  "  ___  ___ ___\n" \
                        " / __|/ __| _ \\\n" \
                        " \\__ \\ (__|  _/\n" \
                        " |___/\\___|_|\n" \
                        "System Control Processor\n"
/*!
 * \brief Standard MCP banner string.
 *
 * \hideinitializer
 */
#define FWK_BANNER_MCP  "  __  __  ___ ___\n" \
                        " |  \\/  |/ __| _ \\\n" \
                        " | |\\/| | (__|  _/\n" \
                        " |_|  |_|\\___|_|\n" \
                        "Manageability Control Processor\n"

/*!
 * \brief Standard ROM firmware banner string.
 */
#define FWK_BANNER_ROM_FIRMWARE "Entered ROM Firmware\n"

/*!
 * \brief Standard RAM firmware banner string.
 */
#define FWK_BANNER_RAM_FIRMWARE "Entered RAM Firmware\n"

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* FWK_BANNER_H */
