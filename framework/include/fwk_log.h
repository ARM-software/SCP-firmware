/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_LOG_H
#define FWK_LOG_H

#include <fwk_macros.h>

#if __has_include(<fmw_log.h>)
#    include <fmw_log.h>
#endif

/*!
 * \addtogroup GroupLibFramework Framework
 * \{
 */

/*!
 * \addtogroup GroupLogging Logging
 *
 *  \details This framework component provides logging facilities to the
 *      firmware. It is intended to be simple and flexible to enable rapid
 *      prototyping of debug-quality code, and performant release-quality code.
 *
 *      This component provides five filter levels for logging messages.
 *      Log messages are assigned a filter level based on the logging macro
 *      used. These macros are as follows:
 *
 *       - ::FWK_LOG_TRACE
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
 *      The logging framework component supports two different backends: the
 *      always-on backend, which provides basic logging facilities for very
 *      early logging, and the dynamic backend, which is intended to
 *      supplant the always-on backend at runtime to provide a more flexible
 *      implementation. These backends are registered through
 *      ::fwk_log_register_aon() and ::fwk_log_register().
 *
 *      An always-on backend can be registered by any firmware at a very early
 *      stage of the boot-up process, including before initialization of
 *      individual modules. This backend is intended to provide a bridge to a
 *      simple always-on device, such as a UART, that can be used as an output
 *      for log messages before a dynamic backend has been registered. See the
 *      documentation for ::fwk_log_register_aon() for more information.
 *
 *      A dynamic backend should be registered by a module, and is expected to
 *      handle more complex scenarios more robustly. This backend may choose to
 *      support multiple log devices of its own, or may support devices that are
 *      not always available. A reference implementation is provided as part of
 *      the standard logging module.
 *
 *      If buffering has been enabled (via ::FMW_LOG_BUFFER_SIZE) then log
 *      messages may be buffered to reduce response latency; these buffered log
 *      messages will be flushed to the dynamic backend once the system has
 *      reached an idle state.
 *
 *      If buffering is enabled, log messages will be buffered if:
 *
 *       - A dynamic backend is currently registered, or
 *       - No backends are currently registered
 *
 *      Buffered log messages will only ever be flushed to the dynamic backend.
 *
 *      Log messages will be written directly to the always-on backend if:
 *
 *       - An always-on backend is currently registered, and
 *       - No dynamic backend is currently registered
 *
 *      If buffering is disabled, log messages will be written directly to the
 *      dynamic backend if one is registered, otherwise to the always-on backend
 *      if one is registered.
 *
 *      In any other case, log messages will be dropped.
 * \{
 */

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
 * \addtogroup GroupLoggingLevels Filter Levels
 *
 * \details The logging framework uses *filter levels* to rank the criticality
 *      of messages, and to filter them if desired. Filtering happens at
 *      preprocessing-time, and consequently filtered messages do not contribute
 *      to the image.
 * \{
 */

/*!
 * \def FWK_LOG_LEVEL_TRACE
 *
 * \brief *Trace* filter level.
 *
 * \details Messages assigned this filter level represent messages used for
 *      tracing logic and debugging.
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
 */

#define FWK_LOG_LEVEL_TRACE 0
#define FWK_LOG_LEVEL_INFO 1
#define FWK_LOG_LEVEL_WARN 2
#define FWK_LOG_LEVEL_ERROR 3
#define FWK_LOG_LEVEL_CRIT 4

/*!
 * \}
 */

/*!
 * \internal
 *
 * \brief Void the result of an expression.
 *
 * \details This is used to prevent the compiler from complaining about unused
 *      variables when filtering logging calls at preprocessing time.
 *
 * \param[in] EXPR The expression to void.
 */
#define FWK_LOG_VOID_EXPR(EXPR) (void)(EXPR);

/*!
 * \internal
 *
 * \brief Void the result of multiple expressions.
 *
 * \details This is used to prevent the compiler from complaining about unused
 *      variables when filtering logging calls at preprocessing time.
 *
 * \param[in] ... The expressions to void.
 */
#define FWK_LOG_VOID(...) \
    do { \
        FWK_MAP(FWK_LOG_VOID_EXPR, __VA_ARGS__) \
    } while (0)

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
 * \def FWK_LOG_TRACE
 *
 * \brief Log a [trace](::FWK_LOG_LEVEL_TRACE) message.
 *
 * \param[in] ... Format string and any associated parameters.
 */

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_TRACE
#    define FWK_LOG_TRACE(...) fwk_log_snprintf(__VA_ARGS__)
#else
#    define FWK_LOG_TRACE(...) FWK_LOG_VOID(__VA_ARGS__)
#endif

/*!
 * \def FWK_LOG_INFO
 *
 * \brief Log an [informational](::FWK_LOG_LEVEL_INFO) message.
 *
 * \param[in] ... Format string and any associated parameters.
 */

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
#    define FWK_LOG_INFO(...) fwk_log_snprintf(__VA_ARGS__)
#else
#    define FWK_LOG_INFO(...) FWK_LOG_VOID(__VA_ARGS__)
#endif

/*!
 * \def FWK_LOG_WARN
 *
 * \brief Log a [warning](::FWK_LOG_LEVEL_WARN) message.
 *
 * \param[in] ... Format string and any associated parameters.
 */

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_WARN
#    define FWK_LOG_WARN(...) fwk_log_snprintf(__VA_ARGS__)
#else
#    define FWK_LOG_WARN(...) FWK_LOG_VOID(__VA_ARGS__)
#endif

/*!
 * \def FWK_LOG_ERR
 *
 * \brief Log an [error](::FWK_LOG_LEVEL_ERROR) message.
 *
 * \param[in] ... Format string and any associated parameters.
 */

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
#    define FWK_LOG_ERR(...) fwk_log_snprintf(__VA_ARGS__)
#else
#    define FWK_LOG_ERR(...) FWK_LOG_VOID(__VA_ARGS__)
#endif

/*!
 * \def FWK_LOG_CRIT
 *
 * \brief Log a [critical](::FWK_LOG_LEVEL_CRIT) message.
 *
 * \param[in] ... Format string and any associated parameters.
 */

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_CRIT
#    define FWK_LOG_CRIT(...) fwk_log_snprintf(__VA_ARGS__)
#else
#    define FWK_LOG_CRIT(...) FWK_LOG_VOID(__VA_ARGS__)
#endif

/*!
 * \brief Logging backend interface.
 *
 * \details This interface represents the medium through which the framework
 *      logging logic transmits data to one or more logging backends.
 */
struct fwk_log_backend {
    /*!
     * \brief Buffer a single character.
     *
     * \note If the backend is not buffered, it may instead choose to print
     *      the character immediately.
     *
     * \param[in] ch Character to buffer.
     *
     * \retval ::FWK_E_DEVICE The operation failed.
     * \retval ::FWK_SUCCESS The operation succeeded.
     *
     * \return Status code representing the result of the operation.
     */
    int (*print)(char ch);

    /*!
     * \brief Flush the backend's buffer (if one exists).
     *
     * \note May be set to \c NULL if the backend is not buffered.
     *
     * \param[in] backend Logging backend.
     *
     * \retval ::FWK_E_DEVICE The operation failed.
     * \retval ::FWK_SUCCESS The operation succeeded.
     *
     * \return Status code representing the result of the operation.
     */
    int (*flush)(void);
};

/*!
 * \brief Register an always-on backend.
 *
 * \details For very early printf-style debugging, an always-on backend may be
 *      registered from within a [constructor function].
 *
 *      [constructor function]:
 * https://developer.arm.com/docs/100067/latest/compiler-specific-function-variable-and-type-attributes/__attribute__constructorpriority-function-attribute
 *
 * \param[in] backend Logging backend to register.
 *
 * \retval ::FWK_E_PARAM One or more parameters were invalid.
 * \retval ::FWK_SUCCESS The operation succeeded.
 *
 * \return Status code representing the result of the operation.
 */
int fwk_log_register_aon(const struct fwk_log_backend *backend);

/*!
 * \brief Deregister the current always-on backend.
 *
 * \details This step is recommended if your logging backend can no longer
 *      supply the facilities required.
 *
 * \return The previously registered backend, or \c NULL if no backend was
 *      registered.
 */
const struct fwk_log_backend *fwk_log_deregister_aon(void);

/*!
 * \brief Register a dynamic backend.
 *
 * \details The dynamic backend is intended to be the primary runtime backend,
 *      and can be used to handle more complex devices such as a UART behind a
 *      power domain.
 *
 * \note If buffering is enabled and a dynamic backend has been
 *      registered, log messages will be buffered.
 *
 * \param[in] backend Logging backend to register.
 *
 * \retval ::FWK_E_PARAM One or more parameters were invalid.
 * \retval ::FWK_SUCCESS The operation succeeded.
 *
 * \return Status code representing the result of the operation.
 */
int fwk_log_register(const struct fwk_log_backend *backend);

/*!
 * \brief Deregister the current dynamic backend.
 *
 * \details This step is recommended if your logging backend can no longer
 *      supply the facilities required.
 *
 * \return The previously registered backend, or \c NULL if no backend was
 *      registered.
 */
const struct fwk_log_backend *fwk_log_deregister(void);

/*!
 * \internal
 *
 * \brief Initialize the logging component.
 *
 * \details This function initializes the logging component, enabling log
 *      messages to be buffered until a backend is registered.
 */
void fwk_log_init(void);

/*!
 * \internal
 *
 * \brief Log a message with a specified filter level.
 *
 * \param[in] format Format string.
 * \param[in] ... Associated parameters.
 */
void fwk_log_snprintf(const char *format, ...)
    __attribute__((format(printf, 1, 2)));

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
 * \}
 */

/*!
 * \}
 */

#endif /* FWK_LOG_H */
