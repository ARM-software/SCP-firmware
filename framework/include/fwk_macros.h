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
 * \internal
 *
 * \brief Return a comma-delimited sequence of numbers from `7` down to `0`.
 *
 * \note This macro is an implementation detail of ::FWK_COUNT_ARGS and is used
 *      as the input to ::__FWK_COUNT_ARGS_N.
 */
#define __FWK_COUNT_ARGS_SEQUENCE 7, 6, 5, 4, 3, 2, 1, 0

/*!
 * \internal
 *
 * \brief Return the value of the *nth* parameter.
 *
 * \note This macro is an implementation detail of ::__FWK_COUNT_ARGS_N_EXPAND.
 *
 * \details This macro does the actual counting of the parameters. It does so
 *      through a bit of trickery that involves taking a concatenation of the
 *      parameters being counted, and a fixed length sequence.
 *
 *      Given a call like the following where the parameters are
 *      `[hello, world]`, the call would look like this (sequence length
 *      adapted for brevity):
 *
 *          __FWK_COUNT_ARGS_N(hello, 3, 2, 1, 0) == 1
 *
 *      This works because every time a new parameter is introduced, the
 *      sequence is shifted one to the right, and therefore the value at `N`
 *      decremented.
 *
 *      You can see this in action by adding another parameter to the example:
 *
 *          __FWK_COUNT_ARGS_N(hello, world, 3, 2, 1, 0) == 2
 *
 *      The number of parameters this can take, of course, is therefore limited
 *      to the length of the sequence.
 */
#define __FWK_COUNT_ARGS_N(_1, _2, _3, _4, _5, _6, _7, N, ...) N

/*!
 * \internal
 *
 * \brief Call ::__FWK_COUNT_ARGS_N after expanding the parameters.
 *
 * \details This macro is part of the implementation of ::FWK_COUNT_ARGS and is
 *      required to properly expand the parameters provided (otherwise the
 *      compiler will complain that we have not provided enough parameters).
 */
#define __FWK_COUNT_ARGS_N_EXPAND(...) __FWK_COUNT_ARGS_N(__VA_ARGS__)

/*!
 * \brief Count the number of arguments provided to the macro.
 *
 * \details This macro counts the number of arguments passed to it, up to a
 *      maximum **7**.
 *
 * \warning This macro does not work with zero parameters.
 *
 * \param[in] ... Parameter list.
 */
#define FWK_COUNT_ARGS(...) \
    __FWK_COUNT_ARGS_N_EXPAND(__VA_ARGS__, __FWK_COUNT_ARGS_SEQUENCE)

/*!
 * \internal
 *
 * \def __FWK_MAP_1
 *
 * \brief Map the first parameter to a value.
 *
 * \param[in] MACRO Macro to apply.
 * \param[in] X Value to apply the macro over.
 *
 * \def __FWK_MAP_2
 * \brief Map the second parameter to a value.
 *
 * \param[in] MACRO Macro to apply.
 * \param[in] X Value to apply the macro over.
 * \param[in] ... Remaining values.
 *
 * \def __FWK_MAP_3
 * \brief Map the third parameter to a value.
 *
 * \param[in] MACRO Macro to apply.
 * \param[in] X Value to apply the macro over.
 * \param[in] ... Remaining values.
 *
 * \def __FWK_MAP_4
 * \brief Map the fourth parameter to a value.
 *
 * \param[in] MACRO Macro to apply.
 * \param[in] X Value to apply the macro over.
 * \param[in] ... Remaining values.
 *
 * \def __FWK_MAP_5
 * \brief Map the fifth parameter to a value.
 *
 * \param[in] MACRO Macro to apply.
 * \param[in] X Value to apply the macro over.
 * \param[in] ... Remaining values.
 *
 * \def __FWK_MAP_6
 * \brief Map the sixth parameter to a value.
 *
 * \param[in] MACRO Macro to apply.
 * \param[in] X Value to apply the macro over.
 * \param[in] ... Remaining values.
 *
 * \def __FWK_MAP_7
 * \brief Map the seventh parameter to a value.
 *
 * \param[in] MACRO Macro to apply.
 * \param[in] X Value to apply the macro over.
 * \param[in] ... Remaining values.
 */

#define __FWK_MAP_1(MACRO, X) MACRO(X)
#define __FWK_MAP_2(MACRO, X, ...) __FWK_MAP_1(MACRO, __VA_ARGS__) MACRO(X)
#define __FWK_MAP_3(MACRO, X, ...) __FWK_MAP_2(MACRO, __VA_ARGS__) MACRO(X)
#define __FWK_MAP_4(MACRO, X, ...) __FWK_MAP_3(MACRO, __VA_ARGS__) MACRO(X)
#define __FWK_MAP_5(MACRO, X, ...) __FWK_MAP_4(MACRO, __VA_ARGS__) MACRO(X)
#define __FWK_MAP_6(MACRO, X, ...) __FWK_MAP_5(MACRO, __VA_ARGS__) MACRO(X)
#define __FWK_MAP_7(MACRO, X, ...) __FWK_MAP_6(MACRO, __VA_ARGS__) MACRO(X)

/*!
 * \internal
 *
 * \brief Get the name of the map macro corresponding to an input of \p N
 *      parameters.
 *
 * \param[in] N Number of parameters.
 */
#define __FWK_MAP_MACRO(N) __FWK_MAP_##N

/*!
 * \internal
 *
 * \brief Map a macro with \p N parameters to a macro \p MACRO.
 *
 * \param[in] N Number of parameters.
 * \param[in] MACRO macro to apply.
 */
#define __FWK_MAP(N, MACRO, ...) __FWK_MAP_MACRO(N)(MACRO, __VA_ARGS__)

/*!
 * \brief Map a macro over a set of input parameters.
 *
 * \details This macro calls a macro \p MACRO, taking one input parameter, for
 *      every other parameter passed.
 *
 * \param[in] MACRO Macro to call.
 * \param[in] ... Input parameters.
 */
#define FWK_MAP(MACRO, ...) \
    __FWK_MAP(FWK_COUNT_ARGS(__VA_ARGS__), MACRO, __VA_ARGS__)

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* FWK_MACROS_H */
