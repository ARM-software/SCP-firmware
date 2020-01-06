/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_noreturn.h>

/*!
 * \brief Arm reset exception handler.
 *
 * \details This is the first function that executes when the core comes online.
 */
noreturn void arm_exception_reset(void);

/*!
 * \brief Invalid exception handler.
 *
 * \details This handler is used as the default in order to catch exceptions
 *      that have not been configured with a handler of their own.
 */
noreturn void arm_exception_invalid(void);
