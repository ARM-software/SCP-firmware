/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Power domain internal state utilities.
 */

#include <internal/power_domain.h>

#include <mod_power_domain.h>

#include <fwk_assert.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <inttypes.h>
#include <stdbool.h>

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
static const char *const default_state_name_table[] = {
    "OFF", "ON", "SLEEP", "3",  "4",  "5",  "6",  "7",
    "8",   "9",  "10",    "11", "12", "13", "14", "15"
};
#endif

bool is_valid_state(const struct pd_ctx *pd, unsigned int state)
{
    return (state < MOD_PD_STATE_COUNT_MAX) &&
        ((pd->valid_state_mask & ((uint32_t)1 << state)) != (uint32_t)0);
}

unsigned int normalize_state(unsigned int state)
{
    enum mod_pd_state state_type = (enum mod_pd_state)state;

    switch (state_type) {
    case MOD_PD_STATE_OFF:
        return (MOD_PD_STATE_COUNT_MAX + 1);

    case MOD_PD_STATE_SLEEP:
        return MOD_PD_STATE_COUNT_MAX;

    default:
        return state;
    }
}

bool is_deeper_state(unsigned int state, unsigned int state_to_compare_to)
{
    return normalize_state(state) > normalize_state(state_to_compare_to);
}

bool is_shallower_state(unsigned int state, unsigned int state_to_compare_to)
{
    return normalize_state(state) < normalize_state(state_to_compare_to);
}

bool is_allowed_by_child(
    const struct pd_ctx *child,
    unsigned int parent_state,
    unsigned int child_state)
{
    if (parent_state >= child->allowed_state_mask_table_size) {
        return false;
    }

    return (
        (child->allowed_state_mask_table[parent_state] &
         ((uint32_t)1 << child_state)) != (uint32_t)0);
}

bool is_allowed_by_children(const struct pd_ctx *pd, unsigned int state)
{
    const struct pd_ctx *child = NULL;
    struct fwk_slist *c_node = NULL;

    FWK_LIST_FOR_EACH(
        &pd->children_list, c_node, struct pd_ctx, child_node, child)
    {
        if (!is_allowed_by_child(child, state, child->requested_state)) {
            return false;
        }
    }

    return true;
}

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
const char *get_state_name(const struct pd_ctx *pd, unsigned int state)
{
    static char const unknown_name[] = "Unknown";

    if (state < pd->config->state_name_table_size) {
        return pd->config->state_name_table[state];
    } else if (state < FWK_ARRAY_SIZE(default_state_name_table)) {
        return default_state_name_table[state];
    } else {
        return unknown_name;
    }
}
#endif

unsigned int number_of_bits_to_shift(uint32_t mask)
{
    unsigned int num_bits = 0;

    if (mask == (uint32_t)0) {
        return 0;
    }

    while ((mask & (uint32_t)1) == (uint32_t)0) {
        mask = mask >> 1;
        num_bits++;
    }

    return num_bits;
}

/* Functions related to a composite state */
unsigned int get_level_state_from_composite_state(
    const uint32_t *table,
    uint32_t composite_state,
    int level)
{
    uint32_t mask = table[level];
    unsigned int shift = number_of_bits_to_shift(mask);

    return (composite_state & mask) >> shift;
}

int get_highest_level_from_composite_state(
    const struct pd_ctx *pd,
    uint32_t composite_state)
{
    uint32_t state;
    unsigned int shift, level;
    const uint32_t *state_mask_table;
    unsigned int table_size;

    if (!pd->cs_support) {
        return 0;
    }

    if (pd->composite_state_levels_mask) {
        shift = number_of_bits_to_shift(pd->composite_state_levels_mask);
        level = (pd->composite_state_levels_mask & composite_state) >> shift;
    } else {
        state_mask_table = pd->composite_state_mask_table;
        table_size = (unsigned int)pd->composite_state_mask_table_size;

        for (level = 0; ((level < table_size) && (pd != NULL));
             level++, pd = pd->parent) {
            state = get_level_state_from_composite_state(
                state_mask_table, composite_state, (int)level);
            if (!is_valid_state(pd, state)) {
                break;
            }
        }
        level--;
    }

    return (int)level;
}

bool is_valid_composite_state(
    struct pd_ctx *target_pd,
    uint32_t composite_state)
{
    unsigned int level, highest_level;
    unsigned int state, child_state = (unsigned int)MOD_PD_STATE_OFF;
    struct pd_ctx *pd = target_pd;
    struct pd_ctx *child = NULL;
    const uint32_t *state_mask_table;
    unsigned int table_size;

    assert(target_pd != NULL);

    if (!pd->cs_support) {
        goto error;
    }

    highest_level = (unsigned int)get_highest_level_from_composite_state(
        pd, composite_state);

    state_mask_table = pd->composite_state_mask_table;
    table_size = (unsigned int)pd->composite_state_mask_table_size;

    if (highest_level >= table_size) {
        goto error;
    }

    for (level = 0; level <= highest_level; level++) {
        if (pd == NULL) {
            goto error;
        }

        state = get_level_state_from_composite_state(
            state_mask_table, composite_state, (int)level);

        if (!is_valid_state(pd, state)) {
            goto error;
        }

        if ((child != NULL) &&
            !is_allowed_by_child(child, state, child_state)) {
            goto error;
        }

        child = pd;
        child_state = state;
        pd = pd->parent;
    }

    return true;

error:
    FWK_LOG_ERR(
        "[PD] Invalid composite state for %s: 0x%" PRIX32,
        fwk_module_get_element_name(target_pd->id),
        composite_state);
    return false;
}

bool is_upwards_transition_propagation(
    const struct pd_ctx *lowest_pd,
    uint32_t composite_state)
{
    int highest_level, level;
    const struct pd_ctx *pd;
    unsigned int state;
    const uint32_t *state_mask_table;

    highest_level =
        get_highest_level_from_composite_state(lowest_pd, composite_state);

    if (!lowest_pd->cs_support) {
        return is_deeper_state(composite_state, lowest_pd->requested_state);
    }

    state_mask_table = lowest_pd->composite_state_mask_table;

    for (level = 0, pd = lowest_pd; (level <= highest_level) && (pd != NULL);
         level++, pd = pd->parent) {
        state = get_level_state_from_composite_state(
            state_mask_table, composite_state, level);
        if (state == pd->requested_state) {
            continue;
        }

        return is_deeper_state(state, pd->requested_state);
    }

    return false;
}

bool is_allowed_by_parent_and_children(struct pd_ctx *pd, unsigned int state)
{
    struct pd_ctx *parent, *child = NULL;
    struct fwk_slist *c_node = NULL;

    parent = pd->parent;
    if (parent != NULL) {
        if (!is_allowed_by_child(pd, parent->current_state, state)) {
            return false;
        }
    }

    FWK_LIST_FOR_EACH(
        &pd->children_list, c_node, struct pd_ctx, child_node, child)
    {
        if (!is_allowed_by_child(child, state, child->current_state)) {
            return false;
        }
    }

    return true;
}
