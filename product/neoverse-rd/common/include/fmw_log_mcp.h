/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     MCP RAM firmware logs configuration.
 */

#ifndef FMW_LOG_MCP_H
#define FMW_LOG_MCP_H

/* Enable custom banner */
#define FMW_LOG_CUSTOM_BANNER

#define FMW_LOG_CUSTOM_BANNER_STRING \
    "" \
    " __  __  ___ ___      __ _                          ", \
        "|  \\/  |/ __| _ \\___ / _(_)_ _ _ ____ __ ____ _ _ _ ___ ", \
        "| |\\/| | (__|  _/___|  _| | '_| '  \\ V  V / _` | '_/ -_)", \
        "|_|  |_|\\___|_|     |_| |_|_| |_|_|_\\_/\\_/\\__,_|_| \\___|", "", \
        BUILD_VERSION_DESCRIBE_STRING, "",

#endif /* FMW_LOG_MCP_H */
