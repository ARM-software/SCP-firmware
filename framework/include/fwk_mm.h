/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Memory management.
 */

#ifndef FWK_MM_H
#define FWK_MM_H

#include <fwk_align.h>

#include <stddef.h>

/*!
 * \addtogroup GroupLibFramework Framework
 * @{
 */

/*!
 * \defgroup GroupMM Memory Management
 * @{
 */

/*!
 * \brief Default allocation alignment based on the alignment of max_align_t
 *      on the platform the software is built for.
 */
#define FWK_MM_DEFAULT_ALIGNMENT (alignof(max_align_t))

/*!
 * \brief Allocate a block of memory.
 *
 * \note Memory allocated by this function will be aligned by default on the
 *      value defined by \ref FWK_MM_DEFAULT_ALIGNMENT.
 *
 * \param num Number of items.
 * \param size Item size in bytes.
 *
 * \retval NULL Allocation failed.
 * \return Pointer to a newly-allocated block of memory.
 */
void *fwk_mm_alloc(size_t num, size_t size);

/*!
 * \brief Allocate a block of memory with specified alignment.
 *
 * \param num Number of items.
 * \param size Item size in bytes.
 * \param alignment Memory alignment in bytes. Must be a power of two.
 *
 * \retval NULL Allocation failed.
 * \return Pointer to a newly-allocated block of memory.
 */
void *fwk_mm_alloc_aligned(size_t num, size_t size, unsigned int alignment);

/*!
 * \brief Allocate a block of memory and initialize all its bits to zero.
 *
 * \note Memory allocated by this function will be aligned by default on the
 *      value defined by \ref FWK_MM_DEFAULT_ALIGNMENT.
 *
 * \param num Number of items.
 * \param size Item size in bytes.
 *
 * \retval NULL Allocation failed.
 * \return Pointer to a newly-allocated block of memory.
 */
void *fwk_mm_calloc(size_t num, size_t size);

/*!
 * \brief Allocate a block of memory with specified alignment and initialize
 *      all its bits to zero.
 *
 * \param num Number of items.
 * \param size Item size in bytes.
 * \param alignment Memory alignment in bytes. Must be a power of two.
 *
 * \retval NULL Allocation failed.
 * \return Pointer to a newly-allocated block of memory.
 */
void *fwk_mm_calloc_aligned(size_t num, size_t size, unsigned int alignment);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* FWK_MM_H */
