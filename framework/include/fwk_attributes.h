/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_ATTRIBUTES_H
#define FWK_ATTRIBUTES_H

#include <fwk_macros.h>

/*!
 * \addtogroup GroupLibFramework Framework
 * \{
 */

/*!
 * \defgroup GroupAttributes Attributes
 *
 * \brief Compiler-independent attribute macros.
 *
 * \details This component exposes a number of compiler-specific macros in a
 *      way that they can be used without causing compilation issues on
 *      compilers that do not support them.
 *
 * @warning Where an attribute does not modify the behaviour of a well-formed
 *      program, it may expand to nothing if the compiler in use does not
 *      support it. However, attributes for which a lack of support may
 *      fundamentally alter the program will not be defined on any compiler that
 *      does not support it.
 *
 * \{
 */

/*!
 * \def FWK_HAS_GNU_ATTRIBUTE
 *
 * \brief Check whether the compiler supports a given GNU attribute.
 *
 * \param[in] ATTRIBUTE Attribute name.
 *
 * \returns A value usable in the expression of a preprocessor `if` or `elif`
 *      statement.
 */

#ifdef __has_attribute
#    define FWK_HAS_GNU_ATTRIBUTE(ATTRIBUTE) __has_attribute(ATTRIBUTE)
#else
#    define FWK_HAS_GNU_ATTRIBUTE(ATTRIBUTE) 0
#endif

/*!
 * \def FWK_HAS_MS_ATTRIBUTE
 *
 * \brief Check whether the compiler supports a given Microsoft attribute.
 *
 * \param[in] ATTRIBUTE Attribute name.
 *
 * \returns A value usable in the expression of a preprocessor `if` or `elif`
 *      statement.
 */

#if defined(__has_declspec_attribute) && !defined(__ARMCC_VERSION)
#    define FWK_HAS_MS_ATTRIBUTE(ATTRIBUTE) __has_declspec_attribute(ATTRIBUTE)
#else
#    define FWK_HAS_MS_ATTRIBUTE(ATTRIBUTE) 0
#endif

/*!
 * \def FWK_ALLOC
 *
 * \brief "Allocator" attribute.
 *
 * \details Hints that the pointer returned by this function does not alias any
 *      other pointer, nor do any pointers to valid objects exist within the
 *      storage addressed by the pointer.
 *
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-malloc-function-attribute
 * \see https://docs.microsoft.com/en-us/cpp/cpp/allocator?view=vs-2019
 */

#if FWK_HAS_GNU_ATTRIBUTE(__malloc__)
#    define FWK_ALLOC_1 __attribute__((__malloc__))
#else
#    define FWK_ALLOC_1
#endif

#if FWK_HAS_MS_ATTRIBUTE(allocator)
#    define FWK_ALLOC_2 __declspec(allocator)
#else
#    define FWK_ALLOC_2
#endif

#if FWK_HAS_MS_ATTRIBUTE(restrict)
#    define FWK_ALLOC_3 __declspec(restrict)
#else
#    define FWK_ALLOC_3
#endif

#define FWK_ALLOC FWK_ALLOC_1 FWK_ALLOC_2 FWK_ALLOC_3

/*!
 * \def FWK_ALLOC_SIZE1
 *
 * \brief "Sizing allocator" attribute.
 *
 * \details Hints that the pointer returned by this function has a size given
 *      by a parameter.
 *
 * \param[in] SIZE_POS Position of the size parameter, 1-indexed.
 *
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-alloc_005fsize-function-attribute
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-alloc_005fsize-variable-attribute
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Type-Attributes.html#index-alloc_005fsize-type-attribute
 */

/*!
 * \def FWK_ALLOC_SIZE2
 *
 * \brief "Sizing allocator" attribute.
 *
 * \details Hints that the pointer returned by this function has a size given
 *      by the product of a size and a count parameter.
 *
 * \param[in] SIZE_POS Position of the size parameter, 1-indexed.
 * \param[in] COUNT_POS Position of the count parameter, 1-indexed.
 *
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-alloc_005fsize-function-attribute
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-alloc_005fsize-variable-attribute
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Type-Attributes.html#index-alloc_005fsize-type-attribute
 */

#if FWK_HAS_GNU_ATTRIBUTE(__alloc_size__)
#    define FWK_ALLOC_SIZE1(SIZE_POS) __attribute__((__alloc_size__(SIZE_POS)))
#    define FWK_ALLOC_SIZE2(SIZE_POS, COUNT_POS) \
        __attribute__((__alloc_size__(SIZE_POS, COUNT_POS)))
#else
#    define FWK_ALLOC_SIZE1(SIZE_POS)
#    define FWK_ALLOC_SIZE2(SIZE_POS, COUNT_POS)
#endif

/*!
 * \def FWK_ALLOC_ALIGN
 *
 * \brief "Aligning allocator" attribute.
 *
 * \details Hints that the pointer returned by this function has an alignment
 *      given by a parameter.
 *
 * \param[in] ALIGN_POS Position of the alignment parameter, 1-indexed.
 *
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-alloc_005falign-function-attribute
 */

#if FWK_HAS_GNU_ATTRIBUTE(__alloc_align__)
#    define FWK_ALLOC_ALIGN(ALIGN_POS) \
        __attribute__((__alloc_align__(ALIGN_POS)))
#else
#    define FWK_ALLOC_ALIGN(ALIGN_POS)
#endif

/*!
 * \def FWK_WEAK
 *
 * \brief "Weak" attribute.
 *
 * \details Applies weak linkage to the item.
 *
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-weak-function-attribute
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-weak-variable-attribute
 */

#if FWK_HAS_GNU_ATTRIBUTE(__weak__)
#    define FWK_WEAK __attribute__((__weak__))
#endif

/*!
 * \def FWK_PRINTF
 *
 * \brief "`printf`-like" attribute.
 *
 * \details Hints that the function takes a string literal with the same
 *      semantics as the `printf` format string.
 *
 * \param[in] STR_POS Position of the format string parameter, 1-indexed.
 * \param[in] VA_POS Position of the argument list, 1-indexed.
 *
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-Wformat-3
 */

#if FWK_HAS_GNU_ATTRIBUTE(__format__)
#    define FWK_PRINTF(STR_POS, VA_POS) \
        __attribute__((__format__(printf, STR_POS, VA_POS)))
#else
#    define FWK_PRINTF(STR_POS, VA_POS)
#endif

/*!
 * \def FWK_CONSTRUCTOR
 *
 * \brief "Constructor" attribute.
 *
 * \details Adds the function to the list of functions to be executed before
 *      `main`.
 *
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-constructor-function-attribute
 */

#if FWK_HAS_GNU_ATTRIBUTE(__constructor__)
#    define FWK_CONSTRUCTOR __attribute__((__constructor__))
#endif

/*!
 * \def FWK_PACKED
 *
 * \brief "Packed" attribute.
 *
 * \details Packs the item that this attribute is attached to.
 *
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Type-Attributes.html#index-aligned-type-attribute
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-packed-variable-attribute
 */

#if FWK_HAS_GNU_ATTRIBUTE(__packed__)
#    define FWK_PACKED __attribute__((__packed__))
#endif

/*!
 * \def FWK_SECTION
 *
 * \brief "Linker section" attribute.
 *
 * \details Places the item that this attribute is attached to into a particular
 *      linker section.
 *
 * \param[in] SECTION String literal representing the name of the section.
 *
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-section-function-attribute
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-section-variable-attribute
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Type-Attributes.html#index-packed-type-attribute
 */

#if FWK_HAS_GNU_ATTRIBUTE(__section__)
#    define FWK_SECTION(SECTION) __attribute__((__section__(SECTION)))
#endif

/*!
 * \def FWK_DEPRECATED
 *
 * \brief "Deprecated" attribute.
 *
 * \details Marks the item that this attribute is attached to as deprecated.
 *
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-deprecated-function-attribute
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-deprecated-variable-attribute
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Type-Attributes.html#index-deprecated-type-attribute
 * \see https://gcc.gnu.org/onlinedocs/gcc/Enumerator-Attributes.html#index-deprecated-enumerator-attribute
 */

#if FWK_HAS_GNU_ATTRIBUTE(__deprecated__)
#    define FWK_DEPRECATED __attribute__((__deprecated__))
#else
#    define FWK_DEPRECATED
#endif

/*!
 * \def FWK_NOINLINE
 *
 * \brief "Do not inline" attribute.
 *
 * \details Hints to the compiler that the function should not be inlined.
 *
 * \see https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-noinline-function-attribute
 */

#if FWK_HAS_GNU_ATTRIBUTE(__noinline__)
#    define FWK_NOINLINE __attribute__((__noinline__))
#else
#    define FWK_NOINLINE
#endif

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* FWK_ATTRIBUTES_H */
