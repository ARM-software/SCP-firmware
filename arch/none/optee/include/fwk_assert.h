/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef OPTEE_FWK_ASSERT_H
#define OPTEE_FWK_ASSERT_H

#include <assert.h>
#include_next <fwk_assert.h>

#if !defined(NDEBUG) && !defined(BUILD_TESTS)
/*
 * Redefine fwk_expect() as SCP-firmware implementation expects
 * assert() macro to return a value fwk_expect() can propgate.
 *
 * SCP-firmware documentation regarding fwk_expect():
 * In debug builds, the macro will evaluate the condition and trigger
 * an assertion if it fails. In release builds, or if running tests, the
 * macro will evaluate the condition and discard its result.
 */
#undef fwk_expect
static inline bool _fwk_expect(bool cond, const char *expr,
			       const char *file, const int line,
			       const char *func)
{
	if (!cond) {
		_assert_log(expr, file, line, func);
		_assert_break();
	}

	return true;
}

#define fwk_expect(condition) _fwk_expect(condition, #condition, \
					  __FILE__, __LINE__, __func__)
#endif /* !NDEBUG && !BUILD_TESTS */
#endif /* OPTEE_FWK_ASSERT_H */
