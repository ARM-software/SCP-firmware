/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_TRACE_H
#define FWK_TRACE_H

#include <inttypes.h>
#include <stdint.h>

/*!
 * \addtogroup GroupLibFramework
 * \defgroup GroupTime Trace operations.
 *
 * \details This component adds support for various trace operations.
 *
 * \{
 */

/*!
 * \brief Start trace of an event.
 *
 * \param[in] ID Event ID.
 * \return Status code representing the result of the operation.
 */
#define FWK_TRACE_START(ID) fwk_trace_start(ID)

/*!
 * \brief Finish trace of an event.
 *
 * \param[in] ID Event ID.
 * \return Status code representing the result of the operation.
 */
#define FWK_TRACE_FINISH(ID, MSG) \
    fwk_trace_finish(__FILE__, __func__, __LINE__, ID, MSG)

/*!
 * \brief Calculate the trace overhead.
 *
 * \return Tracing overhead.
 */
#define FWK_TRACE_CALC_OVERHEAD() fwk_trace_calc_overhead()

typedef uint64_t fwk_trace_count_t;
typedef uint32_t fwk_trace_id_t;
#define PRItraceid    PRIu32
#define PRItracecount PRIu64

/*!
 * \brief Trace driver
 */
struct fwk_trace_driver {
    /*! The number of the trace entries. */
    unsigned int trace_entry_count;

    /*!
     * \brief Get current cycle/timestamp count
     *
     * \return trace count
     */
    fwk_trace_count_t (*get_trace_count)(void);

    /*!
     * \brief Report trace entry
     *
     * \param[in] filename File name where end of trace occured.
     * \param[in] func Function where end of trace occured.
     * \param[in] line Line number where end of trace occured.
     * \param[in] id ID of the event which trace ended.
     * \param[in] trace_count Count of how many time units passed along the
     *                        event.
     * \param[in] msg optional message to add to the trace entry.
     *
     */
    void (*report_trace_entry)(
        const char *filename,
        const char *func,
        const unsigned int line,
        const fwk_trace_id_t id,
        const fwk_trace_count_t trace_count,
        const char *msg);
};

/*!
 * \brief Initialize tracing
 * Note: This function has Constructor attribute which adds it to the list of
 * function executes before `main`.
 */
void fwk_trace_init(void);

/*!
 * \brief Calculate the trace overhead.
 *
 * \return Tracing overhead.
 */
fwk_trace_count_t fwk_trace_calc_overhead(void);

/*!
 * \brief Start trace of an event.
 *
 * \param[in] id Event ID.
 * \return Status code representing the result of the operation.
 */
int fwk_trace_start(const fwk_trace_id_t id);

/*!
 * \brief Finish trace of an event.
 *
 * \param[in] filename Name of the file in which the tracing finished.
 *                     use __FILE__ macro.
 * \param[in] line Line number in which the tracing finished.
 *                 use __LINE__ macro.
 * \param[in] func Name of the function in which the fwk_trace_finish is called.
 *                 use __func__ macro.
 * \param[in] id Event ID.
 * \param[in] msg An optional message.
 *
 * \note id must match a started tracing point to report the count spent between
 *       the tracing points. The rest of the params helps in reporting, and it
 *       is left on the driver how it uses them.
 *
 * \return Status code representing the result of the operation.
 */
int fwk_trace_finish(
    const char *filename,
    const char *func,
    const unsigned int line,
    const fwk_trace_id_t id,
    const char *msg);

/*!
 * \brief Register a framework trace driver.
 *
 * \details
 *
 *      This is a weak function provided by the framework that, by default, does
 *      not register a driver, and should be overridden by the firmware if you
 *      wish to provide one.
 *
 *
 * \return Framework trace driver.
 */
struct fwk_trace_driver fmw_trace_driver(void);

/*!
 * \}
 */

#endif /* FWK_TRACE_H */
