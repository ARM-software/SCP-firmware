/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_MM_H
#define ARCH_MM_H

#include <fwk_arch.h>

/*!
 * \brief Initialize the architecture memory management component.
 *
 * \param[out] Memory management configuration data.
 *
 * \retval ::FWK_E_PANIC The operation failed.
 * \retval ::FWK_SUCCESS The operation succeeded.
 *
 * \return Status code representing the result of the operation.
 */
int arch_mm_init(struct fwk_arch_mm_data *data);

#endif /* ARCH_MM_H */
