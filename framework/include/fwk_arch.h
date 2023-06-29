/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Framework API for the architecture layer.
 */

#ifndef FWK_ARCH_H
#define FWK_ARCH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupLibFramework Framework
 * \{
 */

/*!
 * \defgroup GroupArch Architecture Interface
 * \{
 */

/*!
 * \brief Interrupt driver interface.
 *
 * \details The interrupt driver interface allows the framework to provide
 *      architecture-specific handling of the system interrupts.
 */
struct fwk_arch_interrupt_driver {
    /*!
     * \brief Enable interrupts.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     */
    int (*global_enable)(void);

    /*!
     * \brief Disable interrupts.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     */
    int (*global_disable)(void);

    /*!
     * \brief Test whether an interrupt is enabled.
     *
     * \param interrupt Interrupt number.
     * \param [out] enabled \c true if the interrupt is enabled, else \c false.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     */
    int (*is_enabled)(unsigned int interrupt, bool *enabled);

    /*!
     * \brief Enable an interrupt.
     *
     * \param interrupt Interrupt number.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     */
    int (*enable)(unsigned int interrupt);

    /*!
     * \brief Disable an interrupt.
     *
     * \param interrupt Interrupt number.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     */
    int (*disable)(unsigned int interrupt);

    /*!
     * \brief Check if an interrupt is pending.
     *
     * \param interrupt Interrupt number.
     * \param [out] pending \c true if the interrupt is pending, else \c false.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     */
    int (*is_pending)(unsigned int interrupt, bool *pending);

    /*!
     * \brief Set an interrupt as pending.
     *
     * \param interrupt Interrupt number.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     */
    int (*set_pending)(unsigned int interrupt);

    /*!
     * \brief Clear a pending interrupt.
     *
     * \param interrupt Interrupt number.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     */
    int (*clear_pending)(unsigned int interrupt);

    /*!
     * \brief Set an IRQ interrupt service routine.
     *
     * \param interrupt Interrupt number.
     * \param isr Pointer to the interrupt service routine function.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     */
    int (*set_isr_irq)(unsigned int interrupt, void (*isr)(void));

    /*!
     * \brief Set an IRQ interrupt service routine that should receive a
     *     parameter.
     *
     * \param interrupt Interrupt number.
     * \param isr Pointer to the interrupt service routine function.
     * \param parameter Parameter that should be passed to the isr function.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     */
    int (*set_isr_irq_param)(unsigned int interrupt,
                             void (*isr)(uintptr_t param),
                             uintptr_t parameter);

    /*!
     * \brief Set the interrupt service routine for the non-maskable interrupt
     *      (NMI).
     *
     * \param isr Pointer to the NMI interrupt service routine function.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     */
    int (*set_isr_nmi)(void (*isr)(void));

    /*!
     * \brief Set the interrupt service routine for the non-maskable interrupt
     *      (NMI) that should receive a parameter.
     *
     * \param isr Pointer to the NMI interrupt service routine function.
     * \param parameter Parameter that should be passed to the isr function.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     */
    int (*set_isr_nmi_param)(void (*isr)(uintptr_t param), uintptr_t parameter);

    /*!
     * \brief Set the fault interrupt service routine.
     *
     * \param isr Pointer to the fault interrupt service routine function.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     */
    int (*set_isr_fault)(void (*isr)(void));

    /*!
     * \brief Get the interrupt number for the current interrupt service routine
     *      being processed.
     *
     * \param [out] interrupt Interrupt number.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     * \retval ::FWK_E_STATE An interrupt is not currently being serviced.
     */
    int (*get_current)(unsigned int *interrupt);

    /*!
     * \brief Check if in interrupt context.
     *
     * \retval true if in an interrupt context.
     * \retval false not in an interrupt context.
     */
    bool (*is_interrupt_context)(void);
};

/*!
 * \brief Initialization driver interface.
 *
 * \details The initialization driver interface allows the framework to request
 *      low-level (architecture-specific) initialization.
 */
struct fwk_arch_init_driver {
    /*!
     * \brief Interrupt driver initialization.
     *
     * \details This handler is used by the framework library to request the
     *      interrupt driver.
     *
     * \param [out] driver Pointer to an interrupt driver.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \retval ::FWK_E_PARAM The parameter received by the handler is invalid.
     * \retval ::FWK_E_PANIC Unrecoverable initialization error.
     */
    int (*interrupt)(const struct fwk_arch_interrupt_driver **driver);
};

/*!
 * \brief Initialize the framework library.
 *
 * \param driver Pointer to an initialization driver used to perform the
 *      initialization.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM One or more parameters were invalid.
 * \retval ::FWK_E_PANIC Unrecoverable initialization error.
 */
int fwk_arch_init(const struct fwk_arch_init_driver *driver);

/*!
 * \brief Stop the framework library.
 *
 * \details Before terminating the SCP-firmware, the modules and their elements
 * get the opportunity to release or reset some resources.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PANIC Unrecoverable error.
 */
int fwk_arch_deinit(void);

/*!
 * \brief Architecture defined suspend, will wakup on receiving interrupt
 *
 */
void fwk_arch_suspend(void);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* FWK_ARCH_H */
