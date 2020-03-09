/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_ASSERT_H
#define FWK_ASSERT_H

#include <fwk_noreturn.h>

#include <assert.h>
#include <stdbool.h>

#ifndef static_assert
#   define static_assert _Static_assert
#endif

/*!
 * \addtogroup GroupLibFramework
 * \defgroup GroupAssert Assertion Helpers
 * \{
 */

/*!
 * \brief Force a target-dependent, unrecoverable trap.
 *
 * \note If the framework is being built in debug mode then this function will
 *      loop indefinitely in order to facilitate the connection of a debugger
 *      instead of trapping.
 */
noreturn void fwk_trap(void);

/*!
 * \brief Mark a code path as unreachable.
 *
 * \details This function will trap in debug builds, but in release builds
 *      this code path will be marked as unreachable to the optimizer.
 */
noreturn void fwk_unreachable(void);

/*!
 * \brief Expect the success of a condition.
 *
 * \details This function will trap in debug builds if \p condition evaluates to
 *      \c false, but will otherwise do nothing.
 *
 *      Example usage:
 *
 *      \code{.c}
 *      int n = 5;
 *
 *      if (!fwk_expect(n == 42))
 *          return FWK_E_STATE;
 *      \endcode
 *
 *      In this example, \ref fwk_expect() is used to test the value of \c n. In
 *      debug builds, the condition will not hold and the program will trap. In
 *      release builds, the branch will be taken so the error can be properly
 *      handled.
 *
 * \param condition Condition to test.
 *
 * \retval true The expectation held.
 * \retval false The expectation did not hold.
 */
bool fwk_expect(bool condition);

/*!
 * \brief Assert an invariant.
 *
 * \details This function will trap in debug builds if \p condition evaluates to
 *      \c false. Otherwise, it will be asserted to the optimizer that
 *      \p condition will always evaluates to \c true.
 *
 * \param condition Condition to test.
 */
void fwk_assert(bool condition);

/*!
 * \}
 */

#endif /* FWK_ASSERT_H */
