/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_TIME_H
#define FWK_TIME_H

#include <fwk_assert.h>
#include <fwk_id.h>

#include <inttypes.h>
#include <stdint.h>

struct fwk_arch_counter_driver;

/*!
 * \addtogroup GroupLibFramework
 * \defgroup GroupTime Time operations.
 *
 * \details This component adds support for various time operations, including
 *      timestamping and duration calculations. It also provides facilities for
 *      converting between units of time as well as printing them using the
 *      standard framework printing and logging facilities.
 *
 * \{
 */

/*!
 * \brief Create a nanosecond duration.
 *
 * \param[in] N Number of nanoseconds.
 *
 * \return A value of type ::fwk_duration_ns_t representing the duration \p N.
 */
#define FWK_NS(N) ((fwk_duration_ns_t)(N))

/*!
 * \brief Create a nanosecond duration from a microsecond value.
 *
 * \param[in] N Number of microseconds.
 *
 * \return A value of type ::fwk_duration_ns_t representing the duration \p N.
 */
#define FWK_US(N) FWK_NS((N) * UINT64_C(1000))

/*!
 * \brief Create a nanosecond duration from a millisecond value.
 *
 * \param[in] N Number of milliseconds.
 *
 * \return A value of type ::fwk_duration_ns_t representing the duration \p N.
 */
#define FWK_MS(N) FWK_US((N) * UINT64_C(1000))

/*!
 * \brief Create a nanosecond duration from a second value.
 *
 * \param[in] N Number of seconds.
 *
 * \return A value of type ::fwk_duration_ns_t representing the duration \p N.
 */
#define FWK_S(N) FWK_MS((N) * UINT64_C(1000))

/*!
 * \brief Create a nanosecond duration from a minute value.
 *
 * \param[in] N Number of minutes.
 *
 * \return A value of type ::fwk_duration_ns_t representing the duration \p N.
 */
#define FWK_M(N) FWK_S((N) * UINT64_C(60))

/*!
 * \brief Create a nanosecond duration from an hour value.
 *
 * \param[in] N Number of hours.
 *
 * \return A value of type ::fwk_duration_ns_t representing the duration \p N.
 */
#define FWK_H(N) (FWK_M(N * UINT64_C(60)))

/*!
 * \brief Format specifier for ::fwk_duration_ns_t.
 */
#define FWK_PRDNS PRIu64

/*!
 * \brief Format specifier for ::fwk_duration_us_t.
 */
#define FWK_PRDUS PRIu64

/*!
 * \brief Format specifier for ::fwk_duration_ms_t.
 */
#define FWK_PRDMS PRIu64

/*!
 * \brief Format specifier for ::fwk_duration_s_t.
 */
#define FWK_PRDS PRIu64

/*!
 * \brief Format specifier for ::fwk_duration_m_t.
 */
#define FWK_PRDM PRIu32

/*!
 * \brief Format specifier for ::fwk_duration_h_t.
 */
#define FWK_PRDH PRIu32

/*!
 * \brief A timestamp, representing nanoseconds since boot.
 */
typedef uint64_t fwk_timestamp_t;

/*!
 * \brief A duration of time measured in nanoseconds.
 */
typedef uint64_t fwk_duration_ns_t;

/*!
 * \brief A duration of time measured in microseconds.
 */
typedef uint64_t fwk_duration_us_t;

/*!
 * \brief A duration of time measured in milliseconds.
 */
typedef uint64_t fwk_duration_ms_t;

/*!
 * \brief A duration of time measured in seconds.
 */
typedef uint64_t fwk_duration_s_t;

/*!
 * \brief A duration of time measured in minutes.
 */
typedef uint32_t fwk_duration_m_t;

/*!
 * \brief A duration of time measured in hours.
 */
typedef uint32_t fwk_duration_h_t;

/*!
 * \brief Get a timestamp representing the current time.
 *
 * \return Current timestamp.
 */
fwk_timestamp_t fwk_time_current(void);

/*!
 * \brief Convert a timestamp to duration since boot, in nanoseconds.
 *
 * \param[in] timestamp Timestamp.
 *
 * \return Duration of time since boot, in nanoseconds.
 */
fwk_duration_ns_t fwk_time_stamp_duration(fwk_timestamp_t timestamp);

/*!
 * \brief Get the duration of time between two points.
 *
 * \param[in] start Timestamp representing the beginning of the measurement.
 * \param[in] end Timestamp representing the end of the measurement.
 *
 * \return Duration of time between two points, in nanoseconds.
 */
fwk_duration_ns_t fwk_time_duration(fwk_timestamp_t start, fwk_timestamp_t end);

/*!
 * \brief Convert a nanosecond duration to a microsecond duration.
 *
 * \param[in] duration Duration in nanoseconds.
 *
 * \return Duration in microseconds.
 */
fwk_duration_us_t fwk_time_duration_us(fwk_duration_ns_t duration);

/*!
 * \brief Convert a nanosecond duration to a millisecond duration.
 *
 * \param[in] duration Duration in nanoseconds.
 *
 * \return Duration in milliseconds.
 */
fwk_duration_ms_t fwk_time_duration_ms(fwk_duration_ns_t duration);

/*!
 * \brief Convert a nanosecond duration to a second duration.
 *
 * \param[in] duration Duration in nanoseconds.
 *
 * \return Duration in seconds.
 */
fwk_duration_s_t fwk_time_duration_s(fwk_duration_ns_t duration);

/*!
 * \brief Convert a nanosecond duration to a minute duration.
 *
 * \param[in] duration Duration in nanoseconds.
 *
 * \return Duration in minutes.
 */
fwk_duration_m_t fwk_time_duration_m(fwk_duration_ns_t duration);

/*!
 * \brief Convert a nanosecond duration to an hour duration.
 *
 * \param[in] duration Duration in nanoseconds.
 *
 * \return Duration in hours.
 */
fwk_duration_h_t fwk_time_duration_h(fwk_duration_ns_t duration);

/*!
 * \brief Time driver.
 */
struct fwk_time_driver {
    /*!
     * \brief Read the current timestamp value.
     *
     * \param[in] Driver-specific context given by the firmware.
     *
     * \return Current timestamp.
     */
    fwk_timestamp_t (*timestamp)(const void *ctx);
};

/*!
 * \brief Register a framework time driver.
 *
 * \details In order to update timestamp details on demand, the framework needs
 *      to reach out to a particular device. This is done through the time
 *      driver, which should be configured to read from a particular counter in
 *      the system.
 *
 *      This is a weak function provided by the framework that, by default, does
 *      not register a driver, and should be overridden by the firmware if you
 *      wish to provide one.
 *
 * \param[out] ctx Context specific to the driver, provided to calls to the
 *      driver API.
 *
 * \return Framework time driver.
 */
struct fwk_time_driver fmw_time_driver(const void **ctx);

/*!
 * \}
 */

#endif /* FWK_TIME_H */
