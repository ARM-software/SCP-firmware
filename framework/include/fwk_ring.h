/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_RING_H
#define FWK_RING_H

#include <stdbool.h>
#include <stddef.h>

/*!
 * \addtogroup GroupLibFramework Framework
 * \{
 */

/*!
 *
 * \addtogroup GroupRing Ring Buffers
 *
 * \brief Ring buffer interface.
 *
 * \details A ring buffer is a buffer that behaves as though both ends are
 *      connected. It acts like a continuous loop, overwriting old data, by
 *      maintaining extra state.
 *
 * \{
 */

/*!
 * \brief Ring buffer.
 */
struct fwk_ring {
    /*!
     * \brief Internal storage.
     *
     * \details This is the storage within which reads and writes through the
     *      ring buffer interface will take place.
     */
    char *storage;

    /*!
     * \brief Size of ::fwk_ring::storage in bytes.
     */
    size_t capacity;

    /*!
     * \brief Offset of the leading byte of buffered data.
     */
    size_t head;

    /*!
     * \brief Offset of one byte past the trailing byte of buffered data.
     */
    size_t tail;

    /*!
     * \brief Whether the buffer is at full capacity or not.
     */
    bool full;
};

/*!
 * \brief Initialize a ring buffer from existing storage.
 *
 * \param[out] ring Ring buffer to initialize.
 * \param[in] storage Internal storage buffer.
 * \param[in] storage_size Size of \p storage in bytes.
 */
void fwk_ring_init(struct fwk_ring *ring, char *storage, size_t storage_size);

/*!
 * \brief Get the capacity of a ring buffer.
 *
 * \details The capacity represents the number of bytes a ring buffer can hold.
 *
 * \param[in] ring Ring buffer.
 *
 * \return Capacity of \p ring in bytes.
 */
size_t fwk_ring_get_capacity(const struct fwk_ring *ring);

/*!
 * \brief Get the length of a ring buffer.
 *
 * \details The length represents the number of bytes a ring buffer is currently
 *      holding.
 *
 * \param[in] ring Ring buffer.
 *
 * \return Length of \p ring in bytes.
 */
size_t fwk_ring_get_length(const struct fwk_ring *ring);

/*!
 * \brief Get the number of bytes still available in a ring buffer.
 *
 * \param[in] ring Ring buffer.
 *
 * \return Number of remaining bytes in \p ring.
 */
size_t fwk_ring_get_free(const struct fwk_ring *ring);

/*!
 * \brief Get whether a ring buffer is full or not.
 *
 * \param[in] ring Ring buffer.
 *
 * \retval true The ring buffer is full.
 * \retval false The ring buffer is not full.
 */
bool fwk_ring_is_full(const struct fwk_ring *ring);

/*!
 * \brief Get whether a ring buffer is empty or not.
 *
 * \param[in] ring Ring buffer.
 *
 * \retval true The ring buffer is empty.
 * \retval false The ring buffer is not empty.
 */
bool fwk_ring_is_empty(const struct fwk_ring *ring);

/*!
 * \brief Pop data from the beginning of a ring buffer.
 *
 * \details Popping data from the ring buffer will drop it from the buffer.
 *
 * \note \p buffer may be \c NULL, in which case the data is simply discarded.
 *
 * \param[in, out] ring Ring buffer.
 * \param[out] buffer Buffer to write the data to.
 * \param[in] buffer_size Size of \p buffer in bytes.
 *
 * \return Number of bytes written to \p buffer.
 */
size_t fwk_ring_pop(struct fwk_ring *ring, char *buffer, size_t buffer_size);

/*!
 * \brief Peek at data from the beginning of a ring buffer.
 *
 * \param[in, out] ring Ring buffer.
 * \param[out] buffer Buffer to write the data to.
 * \param[in] buffer_size Size of \p buffer in bytes.
 *
 * \return Number of bytes written to \p buffer.
 */
size_t fwk_ring_peek(
    const struct fwk_ring *ring,
    char *buffer,
    size_t buffer_size);

/*!
 * \brief Push data to the end of a ring buffer.
 *
 * \details Writing data past the capacity of the ring buffer will cause the
 *      oldest data to be popped.
 *
 * \param[in, out] ring Ring buffer.
 * \param[in] buffer Buffer to read data from.
 * \param[in] buffer_size Size of \p buffer in bytes.
 *
 * \return Number of bytes written to \p ring.
 */
size_t fwk_ring_push(
    struct fwk_ring *ring,
    const char *buffer,
    size_t buffer_size);

/*!
 * \brief Clear all data from a ring buffer.
 *
 * \param[in, out] ring Ring buffer.
 */
void fwk_ring_clear(struct fwk_ring *ring);

/*!
 * \}
 */

/*!
 * \}
 */

#endif
