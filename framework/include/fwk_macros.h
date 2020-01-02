/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Generic, library-wide macros.
 */

#ifndef FWK_MACROS_H
#define FWK_MACROS_H

/*!
 * \addtogroup GroupLibFramework Framework
 * @{
 */

/*!
 * \defgroup GroupMacros Macros
 * @{
 */

/*!
 * \brief Get the number of elements in an array.
 *
 * \param A Pointer to the array.
 *
 * \return The number of elements in the array.
 */
#define FWK_ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

/*!
 * \brief Aligns a value to the next multiple.
 *
 * \param VALUE Value to be aligned.
 * \param INTERVAL Multiple to align to.
 *
 * \return The nearest multiple which is greater than or equal to VALUE.
 */
#define FWK_ALIGN_NEXT(VALUE, INTERVAL) ((\
    ((VALUE) + (INTERVAL) - 1) / (INTERVAL)) * (INTERVAL))

/*!
 * \brief Aligns a value to the previous multiple.
 *
 * \param VALUE Value to be aligned.
 * \param INTERVAL Multiple to align to.
 *
 * \return The nearest multiple which is smaller than or equal to VALUE.
 */
#define FWK_ALIGN_PREVIOUS(VALUE, INTERVAL) ( \
    ((VALUE) / (INTERVAL)) * (INTERVAL))

/*!
 * \brief Hertz unit.
 */
#define FWK_HZ  (1UL)

/*!
 * \brief Kilohertz unit.
 */
#define FWK_KHZ (1000UL)

/*!
 * \brief Megahertz unit.
 */
#define FWK_MHZ (1000UL * 1000UL)

/*!
 * \brief Gigahertz unit.
 */
#define FWK_GHZ (1000UL * 1000UL * 1000UL)

/*!
 * \brief Kibibyte (2¹⁰) unit.
 *
 * \details Kibibyte (2¹⁰) unit which contrasts with the SI unit KB (10³)
 */
#define FWK_KIB (1024UL)

/*!
 * \brief Mebibyte (2²⁰) unit.
 *
 * \details Mebibyte (2²⁰) unit which contrasts with the SI unit MB (10⁶)
 */
#define FWK_MIB (1024UL * 1024UL)

/*!
 * \brief Gibibyte (2³⁰) unit.
 *
 * \details Gibibyte (2³⁰) unit which contrasts with the SI unit GB (10⁹)
 */
#define FWK_GIB (1024UL * 1024UL * 1024UL)

/*!
 * \brief Tebibyte (2⁴⁰) unit.
 *
 * \details Tebibyte (2⁴⁰) unit which contrasts with the SI unit TB (10¹²)
 */
#define FWK_TIB (1024ULL * 1024ULL * 1024ULL * 1024ULL)

/** @cond */
#define __FWK_STRINGIFY(X) #X
/** @endcond */

/*!
 * \brief Stringify the value of a macro.
 *
 * \param X The macro to be stringified.
 *
 * \return The stringified value.
 */
#define FWK_STRINGIFY(X) __FWK_STRINGIFY(X)

/*!
 * \brief Read-only register.
 *
 * \details This qualifier can be used to describe a memory mapped read-only
 *      register.
 */
#define FWK_R const volatile

/*!
 * \brief Write-only register.
 *
 * \details This qualifier can be used to describe a memory mapped write-only
 *      register.
 */
#define FWK_W volatile

/*!
 * \brief Read/write register.
 *
 * \details This qualifier can be used to describe a memory mapped read/write
 *      register.
 */
#define FWK_RW volatile

/*!
 * \brief Get the smaller of two values.
 *
 * \param a The first value to compare.
 *
 * \param b The second value to compare.
 *
 * \note The __auto_type extension is used to normalize the types and to protect
 * against double evaluation.
 *
 * \return The smallest value from a and b. If both are equal, b is returned.
 */
#define FWK_MIN(a, b) \
    __extension__({ \
        __auto_type _a = (a); \
        __auto_type _b = (b); \
        _a < _b ? _a : _b; \
    })

/*!
 * \brief Get the larger of two values.
 *
 * \param a The first value to compare.
 *
 * \param b The second value to compare.
 *
 * \note The __auto_type extension is used to normalize the types and to protect
 * against double evaluation.
 *
 * \return The largest value from a and b. If both are equal, b is returned.
 */
#define FWK_MAX(a, b) \
    __extension__({ \
        __auto_type _a = (a); \
        __auto_type _b = (b); \
        _a > _b ? _a : _b; \
    })

/*!
 * \brief Firmware version in UINT32_T format
 *
 * \details The macro encodes a 'major, minor and patch' based version data
 * into a 32 bits value using the following schema:
 * [31:24] Major field
 * [23:16] Minor field
 *  [15:0] Patch field
 *
 * \return None.
 */
#define FWK_BUILD_VERSION (((BUILD_VERSION_MAJOR & 0xff) << 24) | \
                           ((BUILD_VERSION_MINOR & 0xff) << 16) | \
                            (BUILD_VERSION_PATCH & 0xffff))

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* FWK_MACROS_H */
