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

#ifndef __USE_ISOC11
#define static_assert _Static_assert
#endif

/*!
 * \addtogroup GroupLibFramework
 * \defgroup GroupAssert Assertion Helpers
 *
 * \details The framework provides a number of assertion helpers to aid in
 *      debugging and documenting code. Most of these trigger a standard library
 *      assertion in some way, but they differ in when.
 *
 *      Choosing the most appropriate assertion helper can be complex, so their
 *      individual purposes are summarised below:
 *
 *      - ::fwk_trap() - it is unsafe to continue at all
 *      - ::fwk_unreachable() - this code is unreachable in normal
 *          operation, and it is unsafe to continue
 *      - ::fwk_unexpected() - this code is unreachable in normal operation,
 *          but it is safe to continue
 *      - ::fwk_assert(condition) - this condition holds true in normal
 *          operation, and it is unsafe to continue
 *      - ::fwk_check(condition) - this condition holds true in normal
 *          operation, but it is safe to continue (statements only)
 *      - ::fwk_expect(condition) - this condition holds true in normal
 *          operation, but it is safe to continue (branch conditions only)
 *
 * \{
 */

/*!
 * \brief Force a target-dependent abnormal program abort.
 *
 * \note The only behaviour guaranteed by this macro is that the program will
 *      terminate abnormally at the point that this macro is called.
 */
#define fwk_trap() __builtin_trap()

/*!
 * \def fwk_unreachable
 *
 * \brief Mark a code path as unreachable.
 *
 * \details In debug builds, this macro will trigger an assertion. In release
 *      builds this code path will be marked as unreachable to the compiler.
 *
 *      #### Example
 *
 *      \code{.c}
 *      int do_something(int x) {
 *          switch (x) {
 *          case 0:
 *              return FWK_E_STATE;
 *
 *          case 10:
 *              return FWK_SUCCESS;
 *
 *          default:
 *              fwk_unreachable();
 *          }
 *      }
 *      \endcode
 *
 *      In this example, the code will assert in a debug build if the
 *      unreachable code is reached. In a release build the behaviour of the
 *      program is undefined.
 */

#ifdef NDEBUG
#    define fwk_unreachable() __builtin_unreachable()
#else
#    define fwk_unreachable() fwk_assert("Unreachable code reached!" && 0)
#endif

/*!
 * \def fwk_unexpected
 *
 * \brief Mark a code path as unexpected.
 *
 * \details Unexpected code paths are paths which the code should never have
 *      taken, but which have associated error handling.
 *
 *      In debug builds, this macro will trigger an assertion. In release
 *      builds, or if running tests, it will do nothing.
 *
 *      #### Example
 *
 *      \code{.c}
 *      if (rand() == 42) {
 *          fwk_unexpected();
 *
 *          return FWK_E_STATE;
 *      }
 *
 *      return FWK_E_SUCCESS;
 *      \endcode
 *
 *      In this example, the code will assert in a debug build if `rand()`
 *      returns `42`. In a release build the expectation will be removed and the
 *      function will return `FWK_E_STATE`.
 */

#if defined(NDEBUG) || defined(BUILD_TESTS)
#    define fwk_unexpected() ((void)0)
#else
#    define fwk_unexpected() fwk_assert("Unexpected code reached!" && 0)
#endif

/*!
 * \def fwk_assert
 *
 * \brief Assert an invariant.
 *
 * \details This macro will pass the condition to the standard C library's
 *      `assert()` macro, which will evaluate the condition and abort the
 *      program if the condition fails.
 *
 *      Unlike `assert()`, this macro will _evaluate_ the condition regardless
 *      of whether assertions are enabled or not.
 *
 * \param condition Condition to test.
 */

#ifdef NDEBUG
#    define fwk_assert(condition) ((void)(condition))
#else
#    define fwk_assert(condition) assert(condition)
#endif

/*!
 * \def fwk_check
 *
 * \brief Expect the success of a condition in a statement.
 *
 * \details In debug builds, the macro will evaluate the condition and trigger
 *      an assertion if it fails. In release builds, or if running tests, the
 *      macro will evaluate the condition and discard its result.
 *
 *      #### Example
 *
 *      \code{.c}
 *      fwk_check(*x > 5);
 *
 *      *x = 42;
 *      \endcode
 *
 *      In this example, the code will assert in a debug build if `*x` is
 *      greater than 5. In a release build the check will be removed and `*x`
 *      will be assigned `42`.
 *
 * \note This macro is similar to ::fwk_expect(), but expands to a statement
 *      rather than an expression, and does not do any branch weighting.
 *
 * \param condition Condition to test.
 */

#if defined(NDEBUG) || defined(BUILD_TESTS)
#    define fwk_check(condition) ((void)(condition))
#else
#    define fwk_check(condition) fwk_assert(condition)
#endif

/*!
 * \def fwk_expect
 *
 * \brief Expect the success of a condition in an expression.
 *
 * \details In debug builds, the macro will evaluate the condition and trigger
 *      an assertion if it fails. In release builds, or if running tests, the
 *      macro will evaluate the condition and discard its result.
 *
 *      #### Example
 *
 *      \code{.c}
 *      if (!fwk_expect(rand() != 42))
 *          return FWK_E_STATE;
 *
 *      return FWK_E_SUCCESS;
 *      \endcode
 *
 *      In this example, the code will assert in a debug build if `rand` is
 *      equal to 42. In a release build the branch will be taken.
 *
 * \param condition Condition to test.
 *
 * \return The value of `condition`.
 */

#ifdef BUILD_OPTEE
/*
 * When building OP-TEE, implementation of assert() macro cannot be
 * used as a condition evaluation hence fwk_expect() wraps helper function
 * _fwk_expect() to produce a nice error trace when the assertion fails.
 */
bool _fwk_expect(bool cond, const char *file, unsigned int line,
		 const char *func);
#define fwk_expect(condition) \
	_fwk_expect((condition), __FILE__, __LINE__, __func__)
#else
#if defined(NDEBUG) || defined(BUILD_TESTS)
#    define fwk_expect(condition) __builtin_expect((condition), 1)
#else
#    define fwk_expect(condition) (fwk_check(condition), 1)
#endif
#endif /* BUILD_OPTEE */

/*!
 * \}
 */

#endif /* FWK_ASSERT_H */
