/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYNQUACER_DEBUG_H
#define SYNQUACER_DEBUG_H

#include <stdbool.h>
#include <stdint.h>
#include <fwk_assert.h>
#include <mod_log.h>
#include <mod_synquacer_system.h>

#ifdef SYNQUACER_LOG_GROUP_ERROR
#define SYNQUACER_DEV_LOG_ERROR(...) \
    synquacer_system_ctx.log_api->log(MOD_LOG_GROUP_ERROR, __VA_ARGS__)
#else
#define SYNQUACER_DEV_LOG_ERROR(...) \
    do {                             \
    } while (0)
#endif

#ifdef SYNQUACER_LOG_GROUP_WARNING
#define SYNQUACER_DEV_LOG_WARNING(...) \
    synquacer_system_ctx.log_api->log(MOD_LOG_GROUP_WARNING, __VA_ARGS__)
#else
#define SYNQUACER_DEV_LOG_WARNING(...) \
    do {                               \
    } while (0)
#endif

#ifdef SYNQUACER_LOG_GROUP_INFO
#define SYNQUACER_DEV_LOG_INFO(...) \
    synquacer_system_ctx.log_api->log(MOD_LOG_GROUP_INFO, __VA_ARGS__)
#else
#define SYNQUACER_DEV_LOG_INFO(...) \
    do {                            \
    } while (0)
#endif

#ifdef SYNQUACER_LOG_GROUP_DEBUG
#define SYNQUACER_DEV_LOG_DEBUG(...) \
    synquacer_system_ctx.log_api->log(MOD_LOG_GROUP_DEBUG, __VA_ARGS__)
#else
#define SYNQUACER_DEV_LOG_DEBUG(...) \
    do {                             \
    } while (0)
#endif

#endif /* SYNQUACER_DEBUG_H */
