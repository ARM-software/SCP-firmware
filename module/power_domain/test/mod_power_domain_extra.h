/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Power domain unit test support.
 */

#ifndef MOD_POWER_DOMAIN_EXTRA_H
#define MOD_POWER_DOMAIN_EXTRA_H

#include <power_domain.h>

#include <mod_power_domain.h>

#include <fwk_id.h>

int pd_driver_set_state(fwk_id_t dev_id, unsigned int state);

int pd_driver_get_state(fwk_id_t dev_id, unsigned int *state);

int pd_driver_reset(fwk_id_t dev_id);

bool pd_driver_deny(fwk_id_t dev_id, unsigned int state);

int pd_driver_prepare_core_for_system_suspend(fwk_id_t dev_id);

int pd_driver_shutdown(
    fwk_id_t dev_id,
    enum mod_pd_system_shutdown system_shutdown);

bool is_valid_state(const struct pd_ctx *pd, unsigned int state);

unsigned int normalize_state(unsigned int state);

bool is_deeper_state(unsigned int state, unsigned int state_to_compare_to);

bool is_shallower_state(unsigned int state, unsigned int state_to_compare_to);

bool is_allowed_by_child(
    const struct pd_ctx *child,
    unsigned int parent_state,
    unsigned int child_state);

bool is_allowed_by_children(const struct pd_ctx *pd, unsigned int state);

const char *get_state_name(const struct pd_ctx *pd, unsigned int state);

unsigned int number_of_bits_to_shift(uint32_t mask);

unsigned int get_level_state_from_composite_state(
    const uint32_t *table,
    uint32_t composite_state,
    int level);

int get_highest_level_from_composite_state(
    const struct pd_ctx *pd,
    uint32_t composite_state);

bool is_valid_composite_state(
    struct pd_ctx *target_pd,
    uint32_t composite_state);

bool is_upwards_transition_propagation(
    const struct pd_ctx *lowest_pd,
    uint32_t composite_state);

bool is_allowed_by_parent_and_children(struct pd_ctx *pd, unsigned int state);

#endif /* MOD_POWER_DOMAIN_EXTRA_H */
