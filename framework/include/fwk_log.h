/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_LOG_H
#define FWK_LOG_H

#include <fwk_attributes.h>
#include <fwk_io.h>
#include <fwk_macros.h>

#if FWK_HAS_INCLUDE(<fmw_log.h>)
#    include <fmw_log.h> /* cppcheck-suppress missingIncludeSystem */
#endif

/*!
 * \addtogroup GroupLibFramework Framework
 * \{
 */

/*!
 * \addtogroup GroupLogging Logging
 *
 *  \details This framework component provides logging facilities to the
 *      firmware. It is intended to be simple and flexible to enable robust
 *      string logging for user consumption.
 *
 *      This component provides five filter levels for logging messages.
 *      Log messages are assigned a filter level based on the logging macro
 *      used. These macros are as follows:
 *
 *       - ::FWK_LOG_DEBUG
 *       - ::FWK_LOG_INFO
 *       - ::FWK_LOG_WARN
 *       - ::FWK_LOG_ERR
 *       - ::FWK_LOG_CRIT
 *
 *      For a brief description of the meaning of each level, see the
 *      documentation for each macro.
 *
 *      The value of the `FWK_LOG_LEVEL` macro, which can be set through the
 *      build system configuration options, determines the minimum level a log
 *      message must be for it to be included in the binary.
 *
 *      If buffering has been enabled then log messages may be buffered to
 *      reduce overall firmware response latency; these buffered log messages
 *      will be flushed once the system has reached an idle state. By default,
 *      buffering is disabled in debug mode and enabled for all platforms in
 *      release mode, but this behaviour can be adjusted by configuring
 *      ::FMW_LOG_BUFFER_SIZE.
 *
 *      The device used for logging can also be adjusted through
 *      ::FMW_LOG_DRAIN_ID. The default behaviour resorts to using the entity
 *      described by ::FMW_IO_STDOUT_ID as the logging device.
 *
 *      If a message is too large to fit into the remaining space of the
 *      internal buffer, the message will be dropped.
 *
 *      Note that log messages are terminated at the column dictated by
 *      ::FMW_LOG_COLUMNS, or the earliest newline.
 * \{
 */

/*!
 * \def FMW_LOG_DRAIN_ID
 *
 * \brief Identifier of the log drain.
 *
 * \details The log drain represents an entity to which logging messages will
 *      be written, and defaults to ::FMW_IO_STDOUT_ID. Replacing the log drain
 *      identifier allows log messages and normal input/output to be separated.
 */
#ifndef FMW_LOG_DRAIN_ID
#    define FMW_LOG_DRAIN_ID FMW_IO_STDOUT_ID
#endif

/*!
 * \def FMW_LOG_BUFFER_SIZE
 *
 * \brief Size of the internal log buffer in bytes.
 *
 * \details Log messages, for reasons of performance, are buffered unless an
 *      always-on backend is currently in use (and a dynamic backend is not).
 *      This definition dictates the size of the buffer, and can be overridden
 *      by each individual firmware through a definition in a `<fmw_log.h>`
 *      header.
 *
 * \note This definition has a default value of four kilobytes in release builds
 *      and zero in debug builds.
 *
 * \note Setting this definition to a value of `0` will disable buffering. If
 *      buffering is disabled, messages will be transmitted immediately.
 */

#ifndef FMW_LOG_BUFFER_SIZE
#    ifdef BUILD_MODE_DEBUG
#        define FMW_LOG_BUFFER_SIZE 0
#    else
#        define FMW_LOG_BUFFER_SIZE (4 * FWK_KIB)
#    endif
#endif

#if FMW_LOG_BUFFER_SIZE > 0
/*!
 * \def FWK_LOG_BUFFERED
 *
 * \brief Determines whether buffering has been enabled within the logging
 *      framework.
 */
#    define FWK_LOG_BUFFERED
#endif

/*!
 * \def FMW_LOG_COLUMNS
 *
 * \brief Number of columns per line the framework will limit itself to.
 *
 * \details Messages printed using any of the logging functions will be limited
 *      to this column count.
 *
 * \note This definition has a default value of `80`.
 */

#ifndef FMW_LOG_COLUMNS
#    define FMW_LOG_COLUMNS 80
#endif

/*!
 * \def FMW_LOG_ENDLINE_STR
 *
 * \brief String representation of the endline.
 *
 * \details This definition has a default value of `"\r\n"`.
 */

#ifndef FMW_LOG_ENDLINE_STR
#    define FMW_LOG_ENDLINE_STR "\r\n"
#endif

/*!
 * \addtogroup GroupLoggingLevels Filter Levels
 *
 * \details The logging framework uses *filter levels* to rank the criticality
 *      of messages, and to filter them if desired. Filtering happens at
 *      preprocessing-time, and consequently filtered messages do not contribute
 *      to the image.
 * \{
 */

/*!
 * \def FWK_LOG_LEVEL_DEBUG
 *
 * \brief *Debug* filter level.
 *
 * \details Messages assigned this filter level represent messages used for
 *      diagnosing problems.
 *
 * \def FWK_LOG_LEVEL_INFO
 *
 * \brief *Informational* log level.
 *
 * \details Messages assigned this filter level represent informational
 *      messages.
 *
 * \def FWK_LOG_LEVEL_WARN
 *
 * \brief *Warning* log level.
 *
 * \details Messages assigned this filter level represent warnings about
 *      possible errors.
 *
 * \def FWK_LOG_LEVEL_ERROR
 *
 * \brief *Error* log level.
 *
 * \details Messages assigned this filter level represent non-fatal errors.
 *
 * \def FWK_LOG_LEVEL_CRIT
 *
 * \brief *Critical* log level.
 *
 * \details Messages assigned this filter level represent fatal errors.
 *
 * \def FWK_LOG_LEVEL_DISABLED
 *
 * \brief *Disabled* log level.
 *
 * \details If a build sets log level to FWK_LOG_LEVEL_DISABLED,
 *      all logs are disabled.
 *
 */

#define FWK_LOG_LEVEL_DEBUG 0
#define FWK_LOG_LEVEL_INFO 1
#define FWK_LOG_LEVEL_WARN 2
#define FWK_LOG_LEVEL_ERROR 3
#define FWK_LOG_LEVEL_CRIT 4
#define FWK_LOG_LEVEL_DISABLED 5

/*!
 * \}
 */

/*!
 * \brief Flush the logging backend.
 *
 * \details Flushing ensures that all data buffered by either the framework or
 *      the logging backend has been fully written to all logging backends
 *      before the function returns.
 *
 * \warning Flushing can be an expensive operation, and should only be used when
 *      you absolutely need to make sure there is no more buffered data (such
 *      as before powering down a logging backend).
 */
#define FWK_LOG_FLUSH() fwk_log_flush()

/*!
 * \def FWK_TRACE
 *
 * \brief Trace a message.
 *
 * \param[in] ... Format string and any associated parameters.
 */

#ifdef FWK_TRACE_ENABLE
#    define FWK_TRACE(...) fwk_log_printf(__VA_ARGS__)
#else
#    define FWK_TRACE(...)
#endif

/*!
 * \def FWK_LOG_DEBUG
 *
 * \brief Log a [debug](::FWK_LOG_LEVEL_DEBUG) message.
 *
 * \param[in] ... Format string and any associated parameters.
 */

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_DEBUG
#    define FWK_LOG_DEBUG(...) fwk_log_printf(__VA_ARGS__)
#else
#    define FWK_LOG_DEBUG(...)
#endif

/*!
 * \def FWK_LOG_INFO
 *
 * \brief Log an [informational](::FWK_LOG_LEVEL_INFO) message.
 *
 * \param[in] ... Format string and any associated parameters.
 */

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
#    define FWK_LOG_INFO(...) fwk_log_printf(__VA_ARGS__)
#else
#    define FWK_LOG_INFO(...)
#endif

/*!
 * \def FWK_LOG_WARN
 *
 * \brief Log a [warning](::FWK_LOG_LEVEL_WARN) message.
 *
 * \param[in] ... Format string and any associated parameters.
 */

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_WARN
#    define FWK_LOG_WARN(...) fwk_log_printf(__VA_ARGS__)
#else
#    define FWK_LOG_WARN(...)
#endif

/*!
 * \def FWK_LOG_ERR
 *
 * \brief Log an [error](::FWK_LOG_LEVEL_ERROR) message.
 *
 * \param[in] ... Format string and any associated parameters.
 */

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
#    define FWK_LOG_ERR(...) fwk_log_printf(__VA_ARGS__)
#else
#    define FWK_LOG_ERR(...)
#endif

/*!
 * \def FWK_LOG_CRIT
 *
 * \brief Log a [critical](::FWK_LOG_LEVEL_CRIT) message.
 *
 * \param[in] ... Format string and any associated parameters.
 */

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_CRIT
#    define FWK_LOG_CRIT(...) fwk_log_printf(__VA_ARGS__)
#else
#    define FWK_LOG_CRIT(...)
#endif

/*!
 * \internal
 *
 * \brief Log a message with a specified filter level.
 *
 * \param[in] format Format string.
 * \param[in] ... Associated parameters.
 */
void fwk_log_printf(const char *format, ...) FWK_PRINTF(1, 2);

/*!
 * \internal
 *
 * \brief Flush the logging backend(s).
 */
void fwk_log_flush(void);

/*!
 * \internal
 *
 * \brief Unbuffer a single character and send it to the logging backend.
 *
 * \details This function is reserved for the framework implementation, and is
 *      used by the scheduler to print opportunistically when idling, and when
 *      flushing to flush the buffer to the logging backend.
 *
 * \retval ::FWK_PENDING The character was unbuffered successfully but there are
 *      still characters remaining in the buffer.
 * \retval ::FWK_SUCCESS The character was unbuffered successfully and the
 *      buffer is now empty.
 * \retval ::FWK_E_DEVICE The backend returned an error.
 *
 * \return Status code representing the result of the operation.
 */
int fwk_log_unbuffer(void);

/*!
 * \internal
 *
 * \brief Initialize the logging component.
 *
 * \details Initializes the logging framework component, making logging
 *      facilities to the framework and, later, modules.
 *
 * \return Status code representing the result of the operation.
 */
int fwk_log_init(void);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* FWK_LOG_H */
