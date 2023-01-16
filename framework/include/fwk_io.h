/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_IO_H
#define FWK_IO_H

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_macros.h>

#include <stdarg.h>
#include <stddef.h>

#if FWK_HAS_INCLUDE(<fmw_io.h>)
#    include <fmw_io.h>
#endif

/*!
 * \addtogroup GroupLibFramework Framework
 * \{
 */

/*!
 * \defgroup GroupIo Input/Output
 *
 * \details This framework component provides low-level input/output facilities
 *      for byte-level read/write operations on streams, where a stream
 *      represents any abstract entity that can accept or return byte data.
 *
 *      The I/O framework is largely modelled on the C standard library
 *      interfaces provided by `<stdio.h>`, except for the key difference that
 *      the I/O framework does not open files, but system entities. A system
 *      entity can be anything with an associated runtime: a module, an element
 *      or a sub-element. This leaves the precise implementation details of
 *      operating on the stream to the entity.
 *
 *      The four key interface functions are ::fwk_io_open, ::fwk_io_read,
 *      ::fwk_io_write, and ::fwk_io_close. These functions operate on streams,
 *      which are created by a call to ::fwk_io_open provided with an identifier
 *      of an entity implementing ::fwk_io_adapter. See the individual
 *      documentation for these functions for their usage documentation.
 *
 *      Creating an I/O-capable entity is a simple process, consisting of
 *      implementing the ::fwk_module::adapter field when creating your module.
 *      See the documentation for the ::fwk_io_adapter structure for more
 *      information.
 *
 *      The framework component also exposes two default streams opened upon
 *      initialization of the component: ::fwk_io_stdin and ::fwk_io_stdout.
 *      These streams are opened on the entities given by ::FMW_IO_STDIN_ID and
 *      ::FMW_IO_STDOUT_ID.
 *
 * \{
 */

struct fwk_io_stream;

/*!
 * \def FMW_IO_STDIN_ID
 *
 * \brief Standard input identifier.
 *
 * \details This macro defines the identifier of the entity responsible for
 *      handling standard input, similar to `stdin` from the standard library.
 *      It represents the primary source of user input.
 */

#ifndef FMW_IO_STDIN_ID
#    define FMW_IO_STDIN_ID FWK_ID_NONE
#endif

/*!
 * \def FMW_IO_STDOUT_ID
 *
 * \brief Standard output identifier.
 *
 * \details This macro defines the identifier of the entity responsible for
 *      handling standard output, similar to `stdout` from the standard library.
 *      It represents the primary destination for user output.
 */

#ifndef FMW_IO_STDOUT_ID
#    define FMW_IO_STDOUT_ID FWK_ID_NONE
#endif

/*!
 * \brief System entity input/output access modes.
 */
enum fwk_io_mode {
    /*!
     * \brief Open the entity for reading.
     *
     * \details This flag indicates that the entity should be opened with
     *      reading capabilities.
     */
    FWK_IO_MODE_READ = (1 << 0),

    /*!
     * \brief Open the entity for writing.
     *
     * \details This flag indicates that the stream should be opened with
     *      writing capabilities.
     */
    FWK_IO_MODE_WRITE = (1 << 1),

    /*!
     * \brief Treat the stream as a binary stream, rather than a text stream.
     *
     * \details This flag indicates that the stream should be treated as a
     *      binary data. How the behaviour differs between text and binary modes
     *      is defined by the system entity that owns the stream adapter.
     */
    FWK_IO_MODE_BINARY = (1 << 2),
};

/*!
 * \brief Stream adapter.
 *
 * \details Stream adapters enable system entities, like modules, elements and
 *      sub-elements, to handle input/output operations in a manner similar to
 *      standard library file streams.
 */
struct fwk_io_adapter {
    /*!
     * \brief Open the entity for input/output operations.
     *
     * \details This function is intended to provide an opportunity for the
     *      system entity to configure any contexts or peripherals required for
     *      later input/output operations.
     *
     *      The `stream` parameter is guaranteed to be non-null.
     *
     * \note This field may be set to a null pointer value if it does not need
     *      to do any extra validation or preparation.
     *
     * \param[in] stream Stream to open.
     *
     * \return Status code representing the result of the operation.
     *
     * \retval ::FWK_SUCCESS The stream was successfully opened.
     */
    int (*open)(const struct fwk_io_stream *stream);

    /*!
     * \brief Read a character from the stream.
     *
     * \details Fetch a single character from the stream. The function may
     *      return ::FWK_PENDING to indicate that there are no more characters
     *      to fetch.
     *
     *      The `stream` and `ch` parameters are guaranteed to be non-null.
     *
     * \param[in] stream Stream to read from.
     * \param[out] ch Character read from the stream.
     *
     * \return Status code representing the result of the operation.
     *
     * \retval ::FWK_SUCCESS A character was successfully read.
     * \retval ::FWK_PENDING There are no more characters to read.
     */
    int (*getch)(const struct fwk_io_stream *stream, char *ch);

    /*!
     * \brief Write a character to the stream.
     *
     * \details Write a single character to the stream.
     *
     *      The `stream` parameter is guaranteed to be non-null.
     *
     * \param[in] stream Stream to write to.
     * \param[in] ch Character to write to the stream.
     *
     * \return Status code representing the result of the operation.
     *
     * \retval ::FWK_SUCCESS A character was successfully written.
     * \retval ::FWK_E_BUSY The resource is currently unavailable and it cannot
     *      accept new characters.
     */
    int (*putch)(const struct fwk_io_stream *stream, char ch);

    /*!
     * \brief Close the stream.
     *
     * \details Close the stream, preventing any further operations from
     *      occurring.
     *
     *      The `stream` parameter is guaranteed to be non-null.
     *
     * \note This field may be set to a null pointer value if nothing is
     *      required to close the stream.
     *
     * \param[in] stream Stream.
     *
     * \return Status code representing the result of the operation.
     */
    int (*close)(const struct fwk_io_stream *stream);
};

/*!
 * \brief Input/output stream.
 *
 * \details This structure represents the context of a system entity opened
 *      for input/output operations.
 */
struct fwk_io_stream {
    /*!
     * \brief Stream adapter.
     *
     * \note This field is set to a null pointer value if the stream has been
     *      closed.
     */
    const struct fwk_io_adapter *adapter;

    /*!
     * \brief Identifier of the system entity.
     *
     * \details This reflects the entity that the stream was opened for, and not
     *      necessarily the owning entity of the stream adapter. For example, if
     *      the stream was opened on a sub-element of module X, while the
     *      adapter would belong to the module, this identifier would represent
     *      the sub-element.
     */
    fwk_id_t id;

    /*!
     * \brief Access mode that the entity was opened with.
     *
     * \details Multiple access modes may be combined, possibly subject to
     *      certain limitations depending on the access mode. An entity may opt
     *      to reject certain modes if it cannot support them.
     */
    enum fwk_io_mode mode;
};

/*!
 * \brief Open an entity for input/output operations.
 *
 * \details Prepares a system entity for handling input/output operations, in a
 *      similar fashion to the standard library `fopen` function. This function
 *      returns a stream on which input/output operations can be done.
 *
 *      Whether an entity may be opened more than once is defined by the entity
 *      itself. Unlike the standard library file operations, these operations
 *      are not buffered.
 *
 *      A system entity may provide an interface for doing input/output
 *      operations by configuring a [stream adapter](::fwk_module::adapter).
 *
 *      Access modes are defined by the ::fwk_io_mode type, and multiple modes
 *      may be used in combination with one another (certain modes may have
 *      stipulations on which other modes they may be used with).
 *
 * \warning Framework text streams do not support wide characters.
 *
 * \param[out] stream Input/output stream.
 * \param[in] id Identifier of the system entity.
 * \param[in] mode Access mode.
 *
 * \return Status code representing the result of the operation.
 *
 * \retval ::FWK_SUCCESS The entity was successfully opened.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `stream` parameter was a null pointer value.
 *      - The `id` parameter was not a valid system entity identifier.
 *      - The `mode` parameter was not a valid access mode.
 * \retval ::FWK_E_SUPPORT The system entity does not support this access mode.
 * \retval ::FWK_E_HANDLER The `stream` adapter encountered an error.
 */
int fwk_io_open(
    struct fwk_io_stream *restrict stream,
    fwk_id_t id,
    enum fwk_io_mode mode);

/*!
 * \brief Reads a character from a stream.
 *
 * \details Reads a character into `ch` from the input stream `stream`, or
 *      returns ::FWK_PENDING if the end of the stream was reached.
 *
 * \param[in] stream Stream to read from.
 * \param[out] ch Storage for the character to read.
 *
 * \return Status code representing the result of the operation.
 *
 * \retval ::FWK_SUCCESS A character was successfully read.
 * \retval ::FWK_PENDING The end of the stream was reached.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `stream` parameter was a null pointer value.
 *      - The `ch` parameter was a null pointer value.
 * \retval ::FWK_E_STATE The `stream` has already been closed.
 * \retval ::FWK_E_SUPPORT The `stream` was not opened with read access.
 * \retval ::FWK_E_HANDLER The `stream` adapter encountered an error.
 */
int fwk_io_getch(
    const struct fwk_io_stream *restrict stream,
    char *restrict ch);

/*!
 * \brief Write a character to a stream.
 *
 * \details Writes a character `ch` to the output stream `stream`. If the driver
 *      is busy, it waits until the resource is freed and available to receive
 *      new characters.
 *
 * \param[in] stream Stream to write to.
 * \param[in] ch Character to write.
 *
 * \return Status code representing the result of the operation.
 *
 * \retval ::FWK_SUCCESS The character was successfully written.
 * \retval ::FWK_E_PARAM The `stream` parameter was a null pointer value.
 * \retval ::FWK_E_STATE The `stream` has already been closed.
 * \retval ::FWK_E_SUPPORT The `stream` was not opened with write access.
 * \retval ::FWK_E_HANDLER The `stream` adapter encountered an error.
 */
int fwk_io_putch(const struct fwk_io_stream *stream, char ch);

/*!
 * \brief Write a character to a stream.
 *
 * \details Writes a character `ch` to the output stream `stream`. If the driver
 *      is busy `FWK_E_BUSY` will be returned.
 *
 * \param[in] stream Stream to write to.
 * \param[in] ch Character to write.
 *
 * \return Status code representing the result of the operation.
 *
 * \retval ::FWK_SUCCESS The character was successfully written.
 * \retval ::FWK_E_BUSY The `stream` resource is currently busy.
 * \retval ::FWK_E_PARAM The `stream` parameter was a null pointer value.
 * \retval ::FWK_E_STATE The `stream` has already been closed.
 * \retval ::FWK_E_SUPPORT The `stream` was not opened with write access.
 * \retval ::FWK_E_HANDLER The `stream` adapter encountered an error.
 */
int fwk_io_putch_nowait(const struct fwk_io_stream *stream, char ch);

/*!
 * \brief Read data from a stream.
 *
 * \details Reads `count` objects into the given array buffer from the input
 *      stream `stream`. The objects are read as if by retrieving a single
 *      `char` `size` times for each object and storing the results in `buffer`
 *      in the order they are read. The `read` parameter is updated with the
 *      number of objects successfully read.
 *
 *      If the `read` parameter is a null pointer value, the function will
 *      return an ::FWK_E_DATA error if the operation completes successfully but
 *      the number of objects read is less than `count`. This error is not
 *      returned if the `read` parameter is not a null pointer value.
 *
 * \param[in] stream Input stream.
 * \param[out] read Number of objects read.
 * \param[out] buffer Pointer to the array of uninitialized objects to write to.
 * \param[in] size Size of each object
 * \param[in] count Number of objects to read.
 *
 * \return Status code representing the result of the operation.
 *
 * \retval ::FWK_SUCCESS The read completed successfully.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `stream` parameter was a null pointer value.
 *      - The `buffer` parameter was a null pointer value.
 * \retval ::FWK_E_STATE The `stream` has already been closed.
 * \retval ::FWK_E_SUPPORT The `stream` was not opened with read access.
 * \retval ::FWK_E_DATA The read succeeded but the buffer was not filled.
 * \retval ::FWK_E_HANDLER The `stream` adapter encountered an error.
 */
int fwk_io_read(
    const struct fwk_io_stream *restrict stream,
    size_t *restrict read,
    void *restrict buffer,
    size_t size,
    size_t count);

/*!
 * \brief Write data to a stream.
 *
 * \details Writes `count` objects from the given array buffer to the output
 *      stream `stream`. The objects are written as if by reinterpreting each
 *      object as an array of `char` and writing them character by character
 *      `size` times for each object, in order. The `written` parameter is
 *      optional, and may be set to a null pointer value.
 *
 * \param[in] stream Output stream.
 * \param[out] written Number of objects written.
 * \param[in] buffer Pointer to the first object in the array to be written.
 * \param[in] size Size of each object.
 * \param[in] count Number of objects to write.
 *
 * \return Status code representing the result of the operation.
 *
 * \retval ::FWK_SUCCESS All the objects were successfully written.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `stream` parameter was a null pointer value.
 *      - The `buffer` parameter was a null pointer value.
 * \retval ::FWK_E_STATE The `stream` has already been closed.
 * \retval ::FWK_E_SUPPORT The `stream` was not opened with write access.
 * \retval ::FWK_E_HANDLER The `stream` adapter encountered an error.
 */
int fwk_io_write(
    const struct fwk_io_stream *restrict stream,
    size_t *restrict written,
    const void *restrict buffer,
    size_t size,
    size_t count);

/*!
 * \brief Write a string to a stream.
 *
 * \details Writes a string, character by character, to an output stream.
 *
 * \param[in] stream Stream to write to.
 * \param[in] str String to write.
 *
 * \return Status code representing the result of the operation.
 *
 * \retval ::FWK_SUCCESS The string was successfully written.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `stream` parameter was a null pointer value.
 *      - The `str` parameter was a null pointer value.
 * \retval ::FWK_E_STATE The `stream` has already been closed.
 * \retval ::FWK_E_SUPPORT The `stream` was not opened with write access.
 * \retval ::FWK_E_HANDLER The `stream` adapter encountered an error.
 */
int fwk_io_puts(
    const struct fwk_io_stream *restrict stream,
    const char *restrict str);

/*!
 * \brief Write a formatted string to a stream.
 *
 * \details Writes a string, formatted with the arguments provided as part of
 *      an argument list, to an output stream. This function behaves as if the
 *      call were to directly use the standard library's `vsprintf` function to
 *      create the final string. The string is written to the stream as though
 *      ::fwk_io_write were called with the final string.
 *
 * \param[in] stream Stream to write to.
 * \param[in] format Format string.
 * \param[in] args Format arguments.
 *
 * \return Status code representing the result of the operation.
 *
 * \retval ::FWK_SUCCESS The formatted string was successfully written.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `stream` parameter was a null pointer value.
 *      - The `format` parameter was a null pointer value.
 * \retval ::FWK_E_NOMEM There is not enough memory to format the string.
 * \retval ::FWK_E_STATE An internal error occurred:
 *      - An error occurred attempting to format the string.
 *      - The `stream` has already been closed.
 * \retval ::FWK_E_SUPPORT The `stream` was not opened with write access.
 * \retval ::FWK_E_HANDLER The `stream` adapter encountered an error.
 */
int fwk_io_vprintf(
    const struct fwk_io_stream *restrict stream,
    const char *restrict format,
    va_list args);

/*!
 * \brief Write a formatted string to a stream.
 *
 * \details Writes a string, formatted with the given arguments, to an output
 *      stream. This function behaves as if the call were to directly use the
 *      standard library's `sprintf` function to create the final string. The
 *      string is written to the stream as though ::fwk_io_write were called
 *      with the final string.
 *
 * \param[in] stream Stream to write to.
 * \param[in] format Format string.
 * \param[in] ...
 *
 * \return Status code representing the result of the operation.
 *
 * \retval ::FWK_SUCCESS The formatted string was successfully written.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `stream` parameter was a null pointer value.
 *      - The `format` parameter was a null pointer value.
 * \retval ::FWK_E_NOMEM There is not enough memory to format the string.
 * \retval ::FWK_E_STATE An internal error occurred:
 *      - An error occurred attempting to format the string.
 *      - The `stream` has already been closed.
 * \retval ::FWK_E_SUPPORT The `stream` was not opened with write access.
 * \retval ::FWK_E_HANDLER The `stream` adapter encountered an error.
 */
int fwk_io_printf(
    const struct fwk_io_stream *restrict stream,
    const char *restrict format,
    ...);

/*!
 * \brief Close a stream.
 *
 * \details Closes the given stream. Whether or not the operation succeeds, the
 *      stream is no longer associated with a system entity. Any operations
 *      using the stream after the stream has been closed will return an error.
 *      Closing an already-closed stream has no effect.
 *
 * \param[in] stream Stream to close.
 *
 * \return Status code representing the result of the operation.
 *
 * \retval ::FWK_SUCCESS The `stream` was successfully closed.
 * \retval ::FWK_E_PARAM The `stream` parameter was a null pointer value.
 * \retval ::FWK_E_HANDLER The `stream` adapter encountered an error.
 */
int fwk_io_close(struct fwk_io_stream *stream);

/*!
 * \brief Standard input stream.
 *
 * \details This stream represents the standard input stream managed by the
 *      framework. This is an open stream with read permissions to the entity
 *      identified by ::FMW_IO_STDIN_ID, or `NULL` if no identifier is given or
 *      there was an error opening the stream
 */
extern struct fwk_io_stream *fwk_io_stdin;

/*!
 * \brief Standard output stream.
 *
 * \details This stream represents the standard output stream managed by the
 *      framework. This is an open stream with write permissions to the entity
 *      identified by ::FMW_IO_STDOUT_ID, or `NULL` if no identifier is given or
 *      there was an error opening the stream.
 */
extern struct fwk_io_stream *fwk_io_stdout;

/*!
 * \internal
 *
 * \brief Initialize the input/output component.
 *
 * \details Initializes the input/output framework component, including
 *      configuring the standard input and output streams.
 *
 * \return Status code representing the result of the operation.
 */
int fwk_io_init(void);

/*!
 * \
 */

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* FWK_IO_H */
