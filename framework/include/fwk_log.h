/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_LOG_H
#define FWK_LOG_H

#include <fwk_macros.h>

#ifdef BUILD_HAS_MOD_LOG
#    include <mod_log.h>
#endif

/*!
 * \addtogroup GroupLibFramework Framework
 * \{
 */

/*!
 * \addtogroup GroupLogging Logging
 * \{
 */

#ifdef BUILD_HAS_MOD_LOG
#    define FWK_LOG_ENABLED
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
 * \internal
 *
 * \def FWK_LOG_IMPL
 *
 * \brief Log a message.
 *
 * \details This macro logs a message with a specified level.
 *
 * \param[in] API Pointer to a logging API implementing ::mod_log_api.
 * \param[in] LEVEL Filter level of the message.
 * \param[in] ... Format string and any associated parameters.
 */

#ifdef FWK_LOG_ENABLED
#    define FWK_LOG_IMPL(API, LEVEL, ...) \
        ((API)->log((enum mod_log_group)(1 << LEVEL), __VA_ARGS__))
#else
#    define FWK_LOG_IMPL(API, LEVEL, ...) FWK_LOG_VOID(API, LEVEL, __VA_ARGS__)
#endif

/*!
 * \def FWK_LOG_FLUSH
 *
 * \brief Flush the logging backend.
 *
 * \details Flushing ensures that all data buffered by either the framework or
 *      the logging backend has been fully written to all logging backends
 *      before the function returns.
 *
 * \warning Flushing can be an expensive operation, and should only be used when
 *      you absolutely need to make sure there is no more buffered data (such
 *      as before powering down a logging backend).
 *
 * \param[in] API API implementing ::mod_log_api.
 */

#ifdef FWK_LOG_ENABLED
#    define FWK_LOG_FLUSH(API) ((API)->flush())
#else
#    define FWK_LOG_FLUSH(API) FWK_LOG_VOID_EXPR(API)
#endif

/*!
 * \def FWK_LOG_TRACE
 *
 * \brief Log a [trace](::FWK_LOG_LEVEL_TRACE) message.
 *
 * \param[in] API Pointer to a logging API implementing ::mod_log_api.
 * \param[in] ... Format string and any associated parameters.
 */

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_TRACE
#    define FWK_LOG_TRACE(API, ...) \
        FWK_LOG_IMPL(API, FWK_LOG_LEVEL_TRACE, __VA_ARGS__)
#else
#    define FWK_LOG_TRACE(API, ...) FWK_LOG_VOID(API, __VA_ARGS__)
#endif

/*!
 * \def FWK_LOG_INFO
 *
 * \brief Log an [informational](::FWK_LOG_LEVEL_INFO) message.
 *
 * \param[in] API Pointer to a logging API implementing ::mod_log_api.
 * \param[in] ... Format string and any associated parameters.
 */

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
#    define FWK_LOG_INFO(API, ...) \
        FWK_LOG_IMPL(API, FWK_LOG_LEVEL_INFO, __VA_ARGS__)
#else
#    define FWK_LOG_INFO(API, ...) FWK_LOG_VOID(API, __VA_ARGS__)
#endif

/*!
 * \def FWK_LOG_WARN
 *
 * \brief Log a [warning](::FWK_LOG_LEVEL_WARN) message.
 *
 * \param[in] API Pointer to a logging API implementing ::mod_log_api.
 * \param[in] ... Format string and any associated parameters.
 */

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_WARN
#    define FWK_LOG_WARN(API, ...) \
        FWK_LOG_IMPL(API, FWK_LOG_LEVEL_WARN, __VA_ARGS__)
#else
#    define FWK_LOG_WARN(API, ...) FWK_LOG_VOID(API, __VA_ARGS__)
#endif

/*!
 * \def FWK_LOG_ERR
 *
 * \brief Log an [error](::FWK_LOG_LEVEL_ERROR) message.
 *
 * \param[in] API Pointer to a logging API implementing ::mod_log_api.
 * \param[in] ... Format string and any associated parameters.
 */

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
#    define FWK_LOG_ERR(API, ...) \
        FWK_LOG_IMPL(API, FWK_LOG_LEVEL_ERROR, __VA_ARGS__)
#else
#    define FWK_LOG_ERR(API, ...) FWK_LOG_VOID(API, __VA_ARGS__)
#endif

/*!
 * \def FWK_LOG_CRIT
 *
 * \brief Log a [critical](::FWK_LOG_LEVEL_CRIT) message.
 *
 * \param[in] API Pointer to a logging API implementing ::mod_log_api.
 * \param[in] ... Format string and any associated parameters.
 */

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_CRIT
#    define FWK_LOG_CRIT(API, ...) \
        FWK_LOG_IMPL(API, FWK_LOG_LEVEL_CRIT, __VA_ARGS__)
#else
#    define FWK_LOG_CRIT(API, ...) FWK_LOG_VOID(API, __VA_ARGS__)
#endif

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* FWK_LOG_H */
