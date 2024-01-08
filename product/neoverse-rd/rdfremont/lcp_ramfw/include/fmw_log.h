/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     RAM firmware logs configuration.
 */

#ifndef FMW_LOG_H
#define FMW_LOG_H

/* Enable custom banner */
#define FMW_LOG_CUSTOM_BANNER

#define FMW_LOG_CUSTOM_BANNER_STRING \
    "" \
    " _    ___ ___      __ _                              ", \
        "| |  / __| _ \\___ / _(_)_ _ _ ____ __ ____ _ _ _ ___ ", \
        "| |_| (__|  _/___|  _| | '_| '  \\ V  V / _` | '_/ -_)", \
        "|____\\___|_|     |_| |_|_| |_|_|_\\_/\\_/\\__,_|_| \\___|", "", \
        BUILD_VERSION_DESCRIBE_STRING, "",

#endif /* FMW_LOG_H */
