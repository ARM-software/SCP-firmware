/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_MATH_H
#define FWK_MATH_H

#include <limits.h>
#include <stdint.h>

/*!
 * \ingroup GroupLibFramework
 * \defgroup GroupMath Math
 *
 * \details Math helper functions to implement several common operations in an
 *      efficient way.
 *
 * \{
 */

/*!
 * \brief Raise two to an exponent.
 *
 * \param exp The exponent to raise to.
 *
 * \return Two raised to \p exp.
 *
 * \note The type of the value returned by this macro is that of the \p exp
 *      parameter.
 */
#define fwk_math_pow2(exp) (((__typeof__(exp))1) << (exp))

/*!
 * \brief Count the leading zeros of an unsigned integer.
 *
 * \param num Operation input. The result is undefined if \p num is \c 0.
 *
 * \return Number of leading zeroes in the \p num parameter.
 *
 * \note The type of the value returned by this macro is that of the \p num
 *      parameter.
 */
#define fwk_math_clz(num) _Generic((num), \
    unsigned int: __builtin_clz, \
    unsigned long: __builtin_clzl, \
    unsigned long long: __builtin_clzll)(num)

/*!
 * \brief Calculate the binary logarithm (log2) of an integer value.
 *
 * \param num Operation input. The result is undefined if \p num is \c 0.
 *
 * \return The binary logarithm of \p num.
 *
 * \note The type of the value returned by this macro is that of the \p num
 *      parameter.
 * \warning If \p num is not a power of two, the result of this call is rounded
 *      down to the next whole number.
 */
#define fwk_math_log2(num) \
    ((sizeof(num) * CHAR_BIT) - fwk_math_clz(num) - 1)

/*!
 * \}
 */

#endif /* FWK_MATH_H */
