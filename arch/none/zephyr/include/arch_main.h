/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_MAIN_H
#define ARCH_MAIN_H

/*!
 * \brief Initialize the architecture.
 *
 */
int scmi_arch_init(void);

/*!
 * \brief Stop the architecture.
 *
 */
int scmi_arch_deinit(void);


#endif /* ARCH_MAIN_H */
