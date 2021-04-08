/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_ASSERT_H
#define FWK_ASSERT_H

#include <fwk_attributes.h>
#include <fwk_noreturn.h>

#include <assert.h>
#include <stdbool.h>

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

#if defined(NDEBUG) || defined(__clang_analyzer__)
#    define fwk_unreachable() __builtin_unreachable()
#else
#    define fwk_unreachable() fwk_assert((bool)false)
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

#if defined(NDEBUG) || defined(BUILD_TESTS) || defined(__clang_analyzer__)
#    define fwk_unexpected() ((void)0)
#else
#    define fwk_unexpected() fwk_assert((bool)false)
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
#    if FWK_HAS_BUILTIN(__builtin_assume)
#        define fwk_assert(condition) \
            do { \
                bool c = (condition); \
                __builtin_assume(c); \
            } while (0)
#    else
#        define fwk_assert(condition) ((void)(condition))
#    endif
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

#if defined(NDEBUG) || defined(BUILD_TESTS)
#    define fwk_expect(condition) (bool)__builtin_expect((condition), 1)
#else
#    define fwk_expect(condition) (bool)(fwk_check(condition), 1)
#endif

/*!
 * \}
 */

#endif /* FWK_ASSERT_H */
