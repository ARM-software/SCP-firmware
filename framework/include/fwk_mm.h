/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_MM_H
#define FWK_MM_H

#include <fwk_attributes.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/*!
 * \addtogroup GroupLibFramework Framework
 * \{
 */

/*!
 * \addtogroup GroupMM Memory Management
 *
 * \brief Memory management interface.
 *
 * \details The memory management interface augments the standard C library
 *      memory allocation functions with a slightly more consistent interface
 *      and stronger portability guarantees across the variety of systems that
 *      the framework supports.
 *
 * \{
 */

/*!
 * \brief Allocates memory for an array of `num` objects of `size`.
 *
 * \details If allocation succeeds, returns a pointer that is suitably aligned
 *      for any object type with fundamental alignment.
 *
 * \details If the product of `num` and `size` is zero, the behavior of
 *      ::fwk_mm_alloc() is implementation-defined.
 *
 * \note ::fwk_mm_alloc() is thread-safe: it behaves as though only accessing
 *      the memory locations visible through its argument, and not any static
 *      storage.
 *
 * \note A previous call to ::fwk_mm_free() or ::fwk_mm_realloc() that
 *      deallocates a region of memory synchronizes-with a call to
 *      ::fwk_mm_alloc() that allocates the same or a part of the same region
 *      of memory. This synchronization occurs after any access to the memory
 *      by the deallocating function and before any access to the memory by
 *      ::fwk_mm_alloc(). There is a single total order of all allocation and
 *      deallocation functions operating on each particular region of memory.
 *
 * \note This function, semantically, behaves identically to `malloc()`.
 *
 * \param[in] num Number of objects.
 * \param[in] size Size of each object.
 *
 * \return On success, returns the pointer to the beginning of newly allocated
 *      memory. To avoid a memory leak, the returned pointer must be
 *      deallocated with ::fwk_mm_free() or ::fwk_mm_realloc().
 *
 * \return On failure, traps.
 */
FWK_LEAF FWK_NOTHROW FWK_ALLOC FWK_ALLOC_SIZE2(1, 2) FWK_WARN_UNUSED
    void *fwk_mm_alloc(size_t num, size_t size);

/*!
 * \brief Allocates memory for an array of `num` objects of `size`. Does not
 *      trap on failure.
 *
 * \details Identical to fwk_mm_alloc(), but does not trap.
 *
 * \param[in] num Number of objects.
 * \param[in] size Size of each object.
 *
 * \return On success, returns the pointer to the beginning of newly allocated
 *      memory. To avoid a memory leak, the returned pointer must be
 *      deallocated with ::fwk_mm_free() or ::fwk_mm_realloc().
 *
 * \return On failure, returns a null pointer.
 */
FWK_LEAF FWK_NOTHROW FWK_ALLOC FWK_ALLOC_SIZE2(1, 2) FWK_WARN_UNUSED
    void *fwk_mm_alloc_notrap(size_t num, size_t size);

/*!
 * \brief Allocates memory for an array of `num` objects of `size` whose
 *      alignment is specified by `alignment`. The product of the `num` and
 *      `size` parameters must be an integral multiple of `alignment`.
 *
 * \details ::fwk_mm_alloc_aligned() is thread-safe: it behaves as though only
 *      accessing the memory locations visible through its argument, and not
 *      any static storage.
 *
 * \details A previous call to ::fwk_mm_free() or ::fwk_mm_realloc() that
 *      deallocates a region of memory synchronizes-with a call to
 *      ::fwk_mm_alloc_aligned() that allocates the same or a part of the same
 *      region of memory. This synchronization occurs after any access to the
 *      memory by the deallocating function and before any access to the memory
 *      by ::fwk_mm_alloc_aligned(). There is a single total order of all
 *      allocation and deallocation functions operating on each particular
 *      region of memory.
 *
 * \note This function, semantically, behaves identically to
 *      `aligned_alloc()`.
 *
 * \param[in] alignment Specifies the alignment. Must be a valid alignment
 *      supported by the implementation.
 * \param[in] num Number of objects.
 * \param[in] size Size of each object. An integral multiple of `alignment`.
 *
 * \return On success, returns the pointer to the beginning of newly allocated
 *      memory. To avoid a memory leak, the returned pointer must be
 *      deallocated with ::fwk_mm_free() or ::fwk_mm_realloc().
 *
 * \return On failure, traps.
 */
FWK_LEAF
FWK_NOTHROW FWK_ALLOC FWK_ALLOC_ALIGN(1) FWK_ALLOC_SIZE2(2, 3) FWK_WARN_UNUSED
    void *fwk_mm_alloc_aligned(size_t alignment, size_t num, size_t size);

/*!
 * \brief Allocates memory for an array of `num` objects of `size` and
 *      initializes all bytes in the allocated storage to zero.
 *
 * \details If allocation succeeds, returns a pointer to the lowest (first)
 *      byte in the allocated memory block that is suitably aligned for any
 *      object type.
 *
 * \details If the product of `num` and `size` is zero, the behavior is
 *      implementation-defined.
 *
 * \note ::fwk_mm_calloc() is thread-safe: it behaves as though only accessing
 *      the memory locations visible through its argument, and not any static
 *      storage.
 *
 * \note A previous call to ::fwk_mm_free() or ::fwk_mm_realloc() that
 *      deallocates a region of memory synchronizes-with a call to
 *      ::fwk_mm_calloc() that allocates the same or a part of the same region
 *      of memory. This synchronization occurs after any access to the memory
 *      by the deallocating function and before any access to the memory by
 *      ::fwk_mm_calloc(). There is a single total order of all allocation and
 *      deallocation functions operating on each particular region of memory.
 *
 * \note This function, semantically, behaves identically to `calloc()`.
 *
 * \param[in] num Number of objects.
 * \param[in] size Size of each object.
 *
 * \return On success, returns the pointer to the beginning of newly allocated
 *      memory. To avoid a memory leak, the returned pointer must be
 *      deallocated with ::fwk_mm_free() or ::fwk_mm_realloc().
 *
 * \return On failure, traps.
 */
FWK_LEAF FWK_NOTHROW FWK_ALLOC FWK_ALLOC_SIZE2(1, 2) FWK_WARN_UNUSED
    void *fwk_mm_calloc(size_t num, size_t size);

/*!
 * \brief Allocates memory for an array of num objects of size whose alignment
 *      is specified by `alignment` and initializes all bytes in the allocated
 *      storage to zero.
 *
 * \details ::fwk_mm_alloc_aligned() is thread-safe: it behaves as though only
 *      accessing the memory locations visible through its argument, and not
 *      any static storage.
 *
 * \details A previous call to ::fwk_mm_free() or ::fwk_mm_realloc() that
 *      deallocates a region of memory synchronizes-with a call to
 *      ::fwk_mm_calloc_aligned() that allocates the same or a part of the same
 *      region of memory. This synchronization occurs after any access to the
 *      memory by the deallocating function and before any access to the memory
 *      by ::fwk_mm_calloc_aligned(). There is a single total order of all
 *      allocation and deallocation functions operating on each particular
 *      region of memory.
 *
 * \param[in] alignment Specifies the alignment. Must be a valid alignment
 *      supported by the implementation.
 * \param[in] num Number of objects.
 * \param[in] size Size of each object.
 *
 * \return On success, returns the pointer to the beginning of newly allocated
 *      memory. To avoid a memory leak, the returned pointer must be
 *      deallocated with ::fwk_mm_free() or ::fwk_mm_realloc().
 *
 * \return On failure, traps.
 */
FWK_LEAF
FWK_NOTHROW FWK_ALLOC FWK_ALLOC_ALIGN(1) FWK_ALLOC_SIZE2(2, 3) FWK_WARN_UNUSED
    void *fwk_mm_calloc_aligned(size_t alignment, size_t num, size_t size);

/*!
 * \brief Reallocates the given area of memory. It must be previously
 *      allocated by ::fwk_mm_alloc, ::fwk_mm_alloc_aligned, ::fwk_mm_calloc,
 *      ::fwk_mm_calloc_aligned, or ::fwk_mm_realloc and not yet freed with a
 *      call to ::fwk_mm_free or ::fwk_mm_realloc. Otherwise, the results are
 *      undefined.
 *
 * \details The reallocation is done by either:
 *      * expanding or contracting the existing area pointed to by `ptr`,
 *          if possible. The contents of the area remain unchanged up to the
 *          lesser of the new and old sizes. If the area is expanded, the
 *          contents of the new part of the array are undefined.
 *      * allocating a new memory block of size `new_size` bytes, copying
 *          memory area with size equal the lesser of the new and the old
 *          sizes, and freeing the old block.
 *
 * \details If there is not enough memory, the old memory block is not freed
 *      and null pointer is returned.
 *
 * \details If `ptr` is `NULL`, the behavior is the same as calling
 *      `fwk_mm_alloc(new_size)`.
 *
 * \details If `new_size` is zero, the behavior is implementation defined (null
 *      pointer may be returned (in which case the old memory block may or may
 *      not be freed), or some non-null pointer may be returned that may not be
 *      used to access storage).
 *
 * \note ::fwk_mm_realloc() is thread-safe: it behaves as though only accessing
 *      the memory locations visible through its argument, and not any static
 *      storage.
 *
 * \note A previous call to ::fwk_mm_free() or ::fwk_mm_realloc() that
 *      deallocates a region of memory synchronizes-with a call to any
 *      allocation function, including ::fwk_mm_realloc() that allocates the
 *      same or a part of the same region of memory. This synchronization
 *      occurs after any access to the memory by the deallocating function and
 *      before any access to the memory by ::fwk_mm_realloc(). There is a
 *      single total order of all allocation and deallocation functions
 *      operating on each particular region of memory.
 *
 * \note This function, semantically, behaves identically to `realloc()`.
 *
 * \param[in] ptr Pointer to the memory area to be reallocated.
 * \param[in] num Number of objects.
 * \param[in] size Size of each object.
 *
 * \return On success, returns the pointer to the beginning of newly allocated
 *      memory. To avoid a memory leak, the returned pointer must be
 *      deallocated with ::fwk_mm_free() or ::fwk_mm_realloc(). The original
 *      pointer `ptr` is invalidated and any access to it is undefined behavior
 *      (even if reallocation was in-place).
 *
 * \return On failure, returns a null pointer. The original pointer `ptr`
 *      remains valid and may need to be deallocated with ::fwk_mm_free() or
 *      ::fwk_mm_realloc().
 */
FWK_LEAF FWK_NOTHROW FWK_ALLOC_SIZE2(2, 3) FWK_WARN_UNUSED
    void *fwk_mm_realloc(void *ptr, size_t num, size_t size);

/*!
 * \brief Deallocates the space previously allocated by ::fwk_mm_alloc(),
 *      ::fwk_mm_calloc(), ::fwk_mm_alloc_aligned(), ::fwk_mm_calloc_aligned(),
 *      or ::fwk_mm_realloc().
 *
 * \details If `ptr` is a null pointer, the function does nothing.
 *
 * \details The behavior is undefined if the value of `ptr` does not equal a
 *      value returned earlier by ::fwk_mm_alloc(), ::fwk_mm_calloc(),
 *      ::fwk_mm_realloc(), ::fwk_mm_alloc_aligned(), or
 *      ::fwk_mm_calloc_aligned().
 *
 * \details The behavior is undefined if the memory area referred to by `ptr`
 *      has already been deallocated, that is, ::fwk_mm_free() or
 *      ::fwk_mm_realloc() has already been called with `ptr` as the argument
 *      and no calls to ::fwk_mm_alloc(), ::fwk_mm_calloc(),
 *      ::fwk_mm_alloc_aligned(), ::fwk_mm_calloc_aligned(), or
 *      ::fwk_mm_realloc() resulted in a pointer equal to `ptr` afterwards.
 *
 * \note This function, semantically, behaves identically to `free()`.
 *
 * \param[in] ptr Pointer to the block of memory to free.
 */
FWK_LEAF FWK_NOTHROW void fwk_mm_free(void *ptr);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* FWK_MM_H */
