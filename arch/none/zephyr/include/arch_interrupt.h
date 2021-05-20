/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_INTERRUPT_H
#define ARCH_INTERRUPT_H

#include <fwk_arch.h>

/*!
 * \brief Initialize the architecture interrupt management component.
 *
 * \param[out] Pointer to the interrupt driver.
 *
 * \retval ::FWK_E_PANIC The operation failed.
 * \retval ::FWK_SUCCESS The operation succeeded.
 *
 * \return Status code representing the result of the operation.
 */
int arch_interrupt_init(const struct fwk_arch_interrupt_driver **driver);

void zephyr_irq_unlock(unsigned int flags);

unsigned int zephyr_irq_lock(void);

#endif /* ARCH_INTERRUPT_H */
