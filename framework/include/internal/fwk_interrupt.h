/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Interrupt management internal resources.
 */

#ifndef INTERNAL_FWK_INTERRUPT_H
#define INTERNAL_FWK_INTERRUPT_H

/*!
 * \addtogroup GroupLibFramework Framework
 * \{
 */

/*!
 * \defgroup GroupInterrupt Interrupt Management
 * \{
 */

/*!
 * \brief Set the fault interrupt service routine. This is used internally by
 *      the framework for test purposes.
 *
 * \param isr Pointer to the fault interrupt service routine function.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM One or more parameters were invalid.
 * \retval ::FWK_E_INIT The component has not been initialized.
 */
int fwk_interrupt_set_isr_fault(void (*isr)(void));

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* INTERNAL_FWK_INTERRUPT_H */
