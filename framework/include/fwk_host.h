/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_HOST_H
#define FWK_HOST_H

#ifdef BUILD_HOST
#include <stdio.h>

/*!
 * \brief Print a message using the host's standard output.
 *
 * \param fmt Const char pointer to the message format string.
 * \param ... Additional arguments for the % specifiers within the message.
 *
 * \return On success, the number of characters written.
 * \return On failure, a negative number containing the error code as per the
 *      printf() specification.
 */
#define FWK_HOST_PRINT printf

#else
#define FWK_HOST_PRINT(...) \
    do { \
    } while (0)
#endif

#endif /* FWK_HOST_H */
