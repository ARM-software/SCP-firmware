/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_module.h>
#include <Mockfwk_notification.h>
#include <Mockmod_power_domain_extra.h>
#include <config_power_domain.h>
#include <power_domain_utils.h>

#include <mod_power_domain_extra.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include UNIT_TEST_SRC

static struct pd_ctx pd_ctx[PD_IDX_COUNT];

struct mod_pd_mod_ctx mod_pd_ctx_temp;

/*
 * Utility functions for initializing the PD context table
 */

static void init_module_ctx(void)
{
    mod_pd_ctx_temp.pd_ctx_table = pd_ctx;
    mod_pd_ctx_temp.pd_count = PD_IDX_COUNT;
    mod_pd_ctx_temp.system_pd_ctx =
        &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_COUNT - 1];

    for (int i = 0; i < PD_IDX_COUNT; ++i) {
        pd_ctx[i] = pd_ctx_config[i];
        fwk_list_init(&pd_ctx[i].children_list);
    }
}

static void evaluate_valid_state_mask(void)
{
    for (int i = 0; i < PD_IDX_COUNT; ++i) {
        for (unsigned int state = 0;
             state < pd_ctx[i].allowed_state_mask_table_size;
             state++) {
            pd_ctx[i].valid_state_mask |=
                pd_ctx[i].allowed_state_mask_table[state];
        }
    }
}

static void construct_pd_relations(void)
{
    pd_ctx[PD_IDX_SYSTOP].parent = NULL;

    fwk_list_push_tail(
        &pd_ctx[PD_IDX_SYSTOP].children_list,
        &pd_ctx[PD_IDX_CLUSTER0].child_node);
    fwk_list_push_tail(
        &pd_ctx[PD_IDX_SYSTOP].children_list,
        &pd_ctx[PD_IDX_CLUSTER1].child_node);
    pd_ctx[PD_IDX_CLUSTER0].parent = &pd_ctx[PD_IDX_SYSTOP];
    pd_ctx[PD_IDX_CLUSTER1].parent = &pd_ctx[PD_IDX_SYSTOP];

    fwk_list_push_tail(
        &pd_ctx[PD_IDX_CLUSTER0].children_list,
        &pd_ctx[PD_IDX_CLUS0CORE0].child_node);
    fwk_list_push_tail(
        &pd_ctx[PD_IDX_CLUSTER0].children_list,
        &pd_ctx[PD_IDX_CLUS0CORE1].child_node);
    pd_ctx[PD_IDX_CLUS0CORE0].parent = &pd_ctx[PD_IDX_CLUSTER0];
    pd_ctx[PD_IDX_CLUS0CORE1].parent = &pd_ctx[PD_IDX_CLUSTER0];

    fwk_list_push_tail(
        &pd_ctx[PD_IDX_CLUSTER1].children_list,
        &pd_ctx[PD_IDX_CLUS1CORE0].child_node);
    fwk_list_push_tail(
        &pd_ctx[PD_IDX_CLUSTER1].children_list,
        &pd_ctx[PD_IDX_CLUS1CORE1].child_node);
    pd_ctx[PD_IDX_CLUS1CORE0].parent = &pd_ctx[PD_IDX_CLUSTER1];
    pd_ctx[PD_IDX_CLUS1CORE1].parent = &pd_ctx[PD_IDX_CLUSTER1];
}

static void change_child_states(struct pd_ctx *pd, unsigned int state)
{
    struct pd_ctx *child = NULL;
    struct fwk_slist *c_node = NULL;

    FWK_LIST_FOR_EACH(
        &pd->children_list, c_node, struct pd_ctx, child_node, child)
    {
        child->requested_state = state;
    }
}

void setUp(void)
{
    memset(pd_ctx, 0, sizeof(pd_ctx));

    init_module_ctx();
    evaluate_valid_state_mask();
    construct_pd_relations();
}

void tearDown(void)
{
    Mockfwk_module_Destroy();
}

void test_is_valid_state_with_valid_state(void)
{
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_SYSTOP];

    int status = is_valid_state(pd, MOD_PD_STATE_ON);
    TEST_ASSERT_EQUAL(true, status);
}

void test_is_valid_state_with_invalid_state(void)
{
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_SYSTOP];
    int status = is_valid_state(pd, MOD_PD_STATE_OFF);
    TEST_ASSERT_EQUAL(false, status);
}

void test_normalize_state(void)
{
    /*
     * Normalize state gives the states new values, as their
     * position in the enum as defined does not represent their
     * level fo hierarchy when comparing their depths.
     */
    unsigned int state = normalize_state(MOD_PD_STATE_SLEEP);
    TEST_ASSERT_EQUAL(MOD_PD_STATE_COUNT_MAX, state);

    state = normalize_state(MOD_PD_STATE_ON);
    TEST_ASSERT_EQUAL(MOD_PD_STATE_ON, state);

    state = normalize_state(MOD_PD_STATE_OFF);
    TEST_ASSERT_EQUAL((MOD_PD_STATE_COUNT_MAX + 1), state);
}

void test_is_deeper_state(void)
{
    bool result = is_deeper_state(MOD_PD_STATE_OFF, MOD_PD_STATE_SLEEP);
    TEST_ASSERT_EQUAL(true, result);

    result = is_deeper_state(MOD_PD_STATE_ON, MOD_PD_STATE_SLEEP);
    TEST_ASSERT_EQUAL(false, result);
}

void test_is_shallower_state(void)
{
    bool result = is_shallower_state(MOD_PD_STATE_SLEEP, MOD_PD_STATE_OFF);
    TEST_ASSERT_EQUAL(true, result);

    result = is_shallower_state(MOD_PD_STATE_SLEEP, MOD_PD_STATE_ON);
    TEST_ASSERT_EQUAL(false, result);
}

void test_is_allowed_by_child_permitted(void)
{
    bool result;
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUSTER0];

    result = is_allowed_by_child(pd, MOD_PD_STATE_ON, MOD_PD_STATE_OFF);
    TEST_ASSERT_EQUAL(true, result);

    pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUS0CORE0];

    result = is_allowed_by_child(pd, MOD_PD_STATE_SLEEP, MOD_PD_STATE_OFF);
    TEST_ASSERT_EQUAL(true, result);
}

void test_is_allowed_by_child_denied(void)
{
    bool result;
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUSTER0];

    result = is_allowed_by_child(pd, MOD_PD_STATE_OFF, MOD_PD_STATE_ON);
    TEST_ASSERT_EQUAL(false, result);

    pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUS0CORE0];

    result = is_allowed_by_child(pd, MOD_PD_STATE_OFF, MOD_PD_STATE_SLEEP);
    TEST_ASSERT_EQUAL(true, result);
}

void test_is_allowed_by_children_permitted(void)
{
    bool result;
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUSTER0];

    change_child_states(pd, MOD_PD_STATE_OFF);

    result = is_allowed_by_children(pd, MOD_PD_STATE_ON);
    TEST_ASSERT_EQUAL(true, result);
}

void test_is_allowed_by_children_denied(void)
{
    bool result;
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUSTER0];

    change_child_states(pd, MOD_PD_STATE_ON);

    result = is_allowed_by_children(pd, MOD_PD_STATE_OFF);
    TEST_ASSERT_EQUAL(false, result);
}

void test_get_state_name_off(void)
{
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUSTER0];
    const char *name = get_state_name(pd, MOD_PD_STATE_OFF);

    TEST_ASSERT_EQUAL("OFF", name);
}

void test_get_state_name_on(void)
{
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUSTER0];
    const char *name = get_state_name(pd, MOD_PD_STATE_ON);

    TEST_ASSERT_EQUAL("ON", name);
}

void test_get_state_name_sleep(void)
{
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUSTER0];
    const char *name = get_state_name(pd, MOD_PD_STATE_SLEEP);

    TEST_ASSERT_EQUAL("SLEEP", name);
}

void test_get_state_name_over_state(void)
{
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUSTER0];
    const char *name = get_state_name(pd, MOD_PD_STATE_COUNT);

    TEST_ASSERT_EQUAL("3", name);
}

void test_num_bits_to_shift(void)
{
    unsigned int number;

    number = number_of_bits_to_shift(0x01);
    TEST_ASSERT_EQUAL(0, number);

    number = number_of_bits_to_shift(0x10);
    TEST_ASSERT_EQUAL(4, number);

    number = number_of_bits_to_shift(0x100);
    TEST_ASSERT_EQUAL(8, number);
}

void test_get_level_state_from_comp_state_on(void)
{
    uint32_t composite_state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON, MOD_PD_STATE_OFF, MOD_PD_STATE_OFF);
    unsigned int state;

    state = get_level_state_from_composite_state(
        core_composite_state_mask_table_UT, composite_state, MOD_PD_LEVEL_2);

    TEST_ASSERT_EQUAL(MOD_PD_STATE_ON, state);
}

void test_get_level_state_from_comp_state_off(void)
{
    uint32_t composite_state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_1, 0, MOD_PD_STATE_ON, MOD_PD_STATE_OFF, MOD_PD_STATE_OFF);
    unsigned int state;

    state = get_level_state_from_composite_state(
        core_composite_state_mask_table_UT, composite_state, MOD_PD_LEVEL_1);

    TEST_ASSERT_EQUAL(MOD_PD_STATE_OFF, state);
}

void test_get_highest_level_from_comp_state(void)
{
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUS0CORE0];
    uint32_t composite_state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON, MOD_PD_STATE_OFF, MOD_PD_STATE_OFF);
    int level;
    pd->cs_support = true;

    level = get_highest_level_from_composite_state(pd, composite_state);

    TEST_ASSERT_EQUAL(MOD_PD_LEVEL_2, level);
}

void test_get_highest_level_from_comp_state_no_cs_support(void)
{
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUS0CORE0];
    uint32_t composite_state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON, MOD_PD_STATE_OFF, MOD_PD_STATE_OFF);
    int level;
    pd->cs_support = false;

    level = get_highest_level_from_composite_state(pd, composite_state);

    TEST_ASSERT_EQUAL(0, level);
}

void test_is_valid_comp_state_true(void)
{
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUS0CORE0];
    uint32_t composite_state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON, MOD_PD_STATE_OFF, MOD_PD_STATE_OFF);
    int valid;
    pd->cs_support = true;

    valid = is_valid_composite_state(pd, composite_state);

    TEST_ASSERT_EQUAL(true, valid);
}

void test_is_valid_comp_state_no_cs_support(void)
{
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUS0CORE0];
    uint32_t composite_state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON, MOD_PD_STATE_OFF, MOD_PD_STATE_OFF);
    int valid;
    pd->cs_support = false;
    fwk_module_get_element_name_ExpectAndReturn(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, PD_IDX_CLUS0CORE0),
        pd_element_table[PD_IDX_CLUS0CORE0].name);

    valid = is_valid_composite_state(pd, composite_state);

    TEST_ASSERT_EQUAL(false, valid);
}

void test_is_valid_comp_state_too_high_level(void)
{
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUSTER0];

    uint32_t composite_state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_COUNT,
        MOD_PD_STATE_SLEEP,
        MOD_PD_STATE_SLEEP,
        MOD_PD_STATE_SLEEP,
        MOD_PD_STATE_SLEEP);
    int valid;
    pd->cs_support = true;

    fwk_module_get_element_name_ExpectAndReturn(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, PD_IDX_CLUSTER0),
        pd_element_table[PD_IDX_CLUSTER0].name);

    valid = is_valid_composite_state(pd, composite_state);

    TEST_ASSERT_EQUAL(false, valid);
}

void test_is_upwards_transition_propagation_true(void)
{
    bool valid;
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUS0CORE0];
    uint32_t composite_state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_2,
        0,
        MOD_PD_STATE_OFF,
        MOD_PD_STATE_OFF,
        MOD_PD_STATE_OFF);

    pd->composite_state_mask_table_size =
        FWK_ARRAY_SIZE(core_composite_state_mask_table_UT);

    pd->cs_support = true;

    /* Change the current requested state so a deeper one can be requested */
    for (int i = 0; i < PD_IDX_COUNT; ++i) {
        pd_ctx[i].requested_state = MOD_PD_STATE_ON;
    }

    valid = is_upwards_transition_propagation(pd, composite_state);
    TEST_ASSERT_EQUAL(true, valid);
}

void test_is_upwards_transition_propagation_false(void)
{
    bool valid;
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUS0CORE0];
    uint32_t composite_state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON, MOD_PD_STATE_ON, MOD_PD_STATE_ON);

    pd->composite_state_mask_table_size =
        FWK_ARRAY_SIZE(core_composite_state_mask_table_UT);

    pd->cs_support = true;

    valid = is_upwards_transition_propagation(pd, composite_state);
    TEST_ASSERT_EQUAL(false, valid);
}

void test_is_upwards_transition_propagation_no_cs_true(void)
{
    bool valid;
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUSTER0];
    uint32_t composite_state = MOD_PD_STATE_OFF;

    pd->cs_support = false;

    /* Swap this round so a deeper state can be requested */
    for (int i = 0; i < PD_IDX_COUNT; ++i) {
        pd_ctx[i].requested_state = MOD_PD_STATE_ON;
    }

    valid = is_upwards_transition_propagation(pd, composite_state);
    TEST_ASSERT_EQUAL(true, valid);
}

void test_is_upwards_transition_propagation_no_cs_false(void)
{
    bool valid;
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUS0CORE0];
    uint32_t composite_state = MOD_PD_STATE_ON;

    pd->cs_support = false;

    valid = is_upwards_transition_propagation(pd, composite_state);
    TEST_ASSERT_EQUAL(false, valid);
}

void test_is_allowed_by_parent_and_children_permitted(void)
{
    bool valid;
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUS0CORE0];

    valid = is_allowed_by_parent_and_children(pd, MOD_PD_STATE_OFF);
    TEST_ASSERT_EQUAL(true, valid);
}

void test_is_allowed_by_parent_and_children_denied(void)
{
    bool valid;
    struct pd_ctx *pd = &mod_pd_ctx_temp.pd_ctx_table[PD_IDX_CLUSTER0];

    /* Swap this round so a pd can deny it is permitted */
    for (int i = 0; i < PD_IDX_COUNT; ++i) {
        pd_ctx[i].current_state = MOD_PD_STATE_ON;
    }

    valid = is_allowed_by_parent_and_children(pd, MOD_PD_STATE_OFF);
    TEST_ASSERT_EQUAL(false, valid);
}

int power_domain_state_checks_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_is_valid_state_with_valid_state);
    RUN_TEST(test_is_valid_state_with_invalid_state);
    RUN_TEST(test_normalize_state);
    RUN_TEST(test_is_deeper_state);
    RUN_TEST(test_is_shallower_state);
    RUN_TEST(test_is_allowed_by_child_permitted);
    RUN_TEST(test_is_allowed_by_child_denied);
    RUN_TEST(test_is_allowed_by_children_permitted);
    RUN_TEST(test_is_allowed_by_children_denied);
    RUN_TEST(test_get_state_name_off);
    RUN_TEST(test_get_state_name_on);
    RUN_TEST(test_get_state_name_sleep);
    RUN_TEST(test_get_state_name_over_state);
    RUN_TEST(test_num_bits_to_shift);
    RUN_TEST(test_get_level_state_from_comp_state_on);
    RUN_TEST(test_get_level_state_from_comp_state_off);
    RUN_TEST(test_get_highest_level_from_comp_state);
    RUN_TEST(test_get_highest_level_from_comp_state_no_cs_support);
    RUN_TEST(test_is_valid_comp_state_true);
    RUN_TEST(test_is_valid_comp_state_no_cs_support);
    RUN_TEST(test_is_valid_comp_state_too_high_level);
    RUN_TEST(test_is_upwards_transition_propagation_true);
    RUN_TEST(test_is_upwards_transition_propagation_false);
    RUN_TEST(test_is_upwards_transition_propagation_no_cs_true);
    RUN_TEST(test_is_upwards_transition_propagation_no_cs_false);
    RUN_TEST(test_is_allowed_by_parent_and_children_permitted);
    RUN_TEST(test_is_allowed_by_parent_and_children_denied);
    return UNITY_END();
}

int main(void)
{
    return power_domain_state_checks_test_main();
}
