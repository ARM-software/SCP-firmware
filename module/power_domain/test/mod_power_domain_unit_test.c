/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_module.h>
#include <Mockfwk_notification.h>
#include <Mockmod_power_domain_extra.h>
#include <config_power_domain.h>
#include <internal/Mockfwk_core_internal.h>
#include <power_domain_utils.h>

#include <mod_power_domain_extra.h>
#include <mod_system_power.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include UNIT_TEST_SRC
static const char *const default_state_name_table[] = {
    "OFF", "ON", "SLEEP", "OFF0", "OFF1", "OFF2"
};
static struct mod_pd_driver_api pd_driver = {
    .set_state = pd_driver_set_state,
    .deny = pd_driver_deny,
};

static struct pd_ctx pd_ctx[PD_IDX_COUNT];

/*
 * Utility functions for initializing the PD context table
 */
static void init_module_ctx(void)
{
    mod_pd_ctx.pd_ctx_table = pd_ctx;
    mod_pd_ctx.pd_count = PD_IDX_COUNT;
    mod_pd_ctx.system_pd_ctx = &mod_pd_ctx.pd_ctx_table[PD_IDX_COUNT - 1];

    for (int i = 0; i < PD_IDX_COUNT; ++i) {
        pd_ctx[i] = pd_ctx_config[i];
        pd_ctx[i].driver_api = &pd_driver;
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

void setUp(void)
{
    memset(pd_ctx, 0, sizeof(pd_ctx));
    init_module_ctx();
    evaluate_valid_state_mask();
    construct_pd_relations();
}

void tearDown(void)
{
    /* To clean up the unused mocked functions set by helper functions */
    Mockfwk_module_Destroy();
    Mockmod_power_domain_extra_Destroy();
    Mockfwk_notification_Destroy();
}

static inline void prepare_mocks_for_set_state_request(
    enum pd_idx pd_idx,
    enum mod_pd_state requested_state,
    bool driver_deny_retval,
    int driver_set_state_retval)
{
    fwk_module_get_element_name_ExpectAndReturn(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, pd_idx),
        pd_element_table[pd_idx].name);

    pd_driver_deny_ExpectAndReturn(
        pd_ctx[pd_idx].driver_id, requested_state, driver_deny_retval);

    pd_driver_set_state_ExpectAndReturn(
        pd_ctx[pd_idx].driver_id, requested_state, driver_set_state_retval);
}

static inline void prepare_state_name(uint32_t state)
{
    char const *name_of_state = default_state_name_table[state];

    get_state_name_ExpectAnyArgsAndReturn(name_of_state);
    get_state_name_ExpectAnyArgsAndReturn(name_of_state);
}

static inline void prepare_allow_tree(
    bool allowed_child,
    bool allowed_children,
    bool allowed_parent_children)
{
    is_allowed_by_child_ExpectAnyArgsAndReturn(allowed_child);
    is_allowed_by_children_ExpectAnyArgsAndReturn(allowed_children);
    is_allowed_by_parent_and_children_ExpectAnyArgsAndReturn(
        allowed_parent_children);
}

void test_set_state_cluster_on_expect_transition_init(void)
{
    struct fwk_event event;
    struct fwk_event resp_event;
    struct pd_set_state_request *req_params =
        (struct pd_set_state_request *)event.params;

    req_params->composite_state = MOD_PD_STATE_ON;

    fwk_notification_notify_IgnoreAndReturn(FWK_SUCCESS);
    is_upwards_transition_propagation_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUSTER0], req_params->composite_state, true);
    get_highest_level_from_composite_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUSTER0], req_params->composite_state, MOD_PD_STATE_ON);

    prepare_mocks_for_set_state_request(
        PD_IDX_CLUSTER0, MOD_PD_STATE_ON, false, FWK_SUCCESS);
    prepare_allow_tree(true, true, true);
    prepare_state_name(MOD_PD_STATE_ON);

    retrieve_mapped_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUSTER0], MOD_PD_STATE_ON, MOD_PD_STATE_ON);

    process_set_state_request(&pd_ctx[PD_IDX_CLUSTER0], &event, &resp_event);

    TEST_ASSERT_EQUAL(MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUSTER0].requested_state);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver);
    /* the current state should be off until the driver reports back */
    TEST_ASSERT_EQUAL(MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUSTER0].current_state);
}

void test_set_state_core_on_while_cluster_off_expect_error_in_resp(void)
{
    struct fwk_event event;
    struct fwk_event resp_event;
    struct pd_set_state_request *req_params =
        (struct pd_set_state_request *)event.params;
    struct pd_set_state_response *resp_params =
        (struct pd_set_state_response *)resp_event.params;

    event.response_requested = true;
    req_params->composite_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].cs_support = false;

    fwk_notification_notify_IgnoreAndReturn(FWK_SUCCESS);
    is_upwards_transition_propagation_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUSTER0], req_params->composite_state, true);
    get_highest_level_from_composite_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUSTER0], req_params->composite_state, MOD_PD_STATE_ON);

    prepare_mocks_for_set_state_request(
        PD_IDX_CLUSTER0, MOD_PD_STATE_ON, false, FWK_SUCCESS);
    prepare_allow_tree(false, true, true);
    process_set_state_request(&pd_ctx[PD_IDX_CLUSTER0], &event, &resp_event);

    TEST_ASSERT_EQUAL(FWK_E_PWRSTATE, resp_params->status);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUSTER0].requested_state);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver);
    TEST_ASSERT_EQUAL(MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUSTER0].current_state);
}

void test_set_state_error_in_initiating_transition(void)
{
    struct fwk_event event;
    struct fwk_event resp_event;
    struct pd_set_state_request *req_params =
        (struct pd_set_state_request *)event.params;
    struct pd_set_state_response *resp_params =
        (struct pd_set_state_response *)resp_event.params;

    event.response_requested = true;
    req_params->composite_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].cs_support = false;

    is_upwards_transition_propagation_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUSTER0], req_params->composite_state, true);
    get_highest_level_from_composite_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUSTER0], req_params->composite_state, MOD_PD_STATE_ON);

    prepare_mocks_for_set_state_request(
        PD_IDX_CLUSTER0, MOD_PD_STATE_ON, true, FWK_SUCCESS);
    prepare_allow_tree(true, true, true);
    prepare_state_name(MOD_PD_STATE_ON);

    process_set_state_request(&pd_ctx[PD_IDX_CLUSTER0], &event, &resp_event);

    TEST_ASSERT_EQUAL(FWK_E_DEVICE, resp_params->status);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUSTER0].requested_state);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver);
    TEST_ASSERT_EQUAL(MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUSTER0].current_state);
}

void test_set_state_failed_transition_while_driver_set(void)
{
    struct fwk_event event;
    struct fwk_event resp_event;
    struct pd_set_state_request *req_params =
        (struct pd_set_state_request *)event.params;
    req_params->composite_state = MOD_PD_STATE_ON;
    fwk_notification_notify_IgnoreAndReturn(FWK_SUCCESS);
    prepare_mocks_for_set_state_request(
        PD_IDX_CLUSTER0, MOD_PD_STATE_ON, false, FWK_E_PWRSTATE);
    is_upwards_transition_propagation_ExpectAnyArgsAndReturn(true);
    get_highest_level_from_composite_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUSTER0], req_params->composite_state, MOD_PD_STATE_ON);
    prepare_allow_tree(true, true, true);
    prepare_state_name(MOD_PD_STATE_ON);

    retrieve_mapped_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUSTER0], MOD_PD_STATE_ON, MOD_PD_STATE_ON);

    process_set_state_request(&pd_ctx[PD_IDX_CLUSTER0], &event, &resp_event);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUSTER0].requested_state);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver);
    /* current state should be off until driver report back */
    TEST_ASSERT_EQUAL(MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUSTER0].current_state);
}

void test_set_state_simple_core_wakup(void)
{
    struct fwk_event report;
    struct pd_power_state_transition_report *report_params =
        (struct pd_power_state_transition_report *)(&report.params);
    struct fwk_event event;
    struct pd_set_state_request *req_params =
        (struct pd_set_state_request *)event.params;
    pd_ctx[PD_IDX_CLUSTER0].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].current_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].driver_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE0].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE0].state_requested_to_driver = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE0].current_state = MOD_PD_STATE_OFF;
    pd_ctx[PD_IDX_CLUS0CORE0].driver_state = MOD_PD_STATE_ON;
    req_params->composite_state = MOD_PD_STATE_OFF;

    report =
        (struct fwk_event){ .source_id = pd_ctx[PD_IDX_CLUS0CORE0].driver_id,
                            .target_id = pd_ctx[PD_IDX_CLUS0CORE0].id,
                            .id = FWK_ID_EVENT(
                                FWK_MODULE_IDX_POWER_DOMAIN,
                                PD_EVENT_IDX_REPORT_POWER_STATE_TRANSITION) };
    report_params->state = MOD_PD_STATE_ON;
    fwk_notification_notify_IgnoreAndReturn(FWK_SUCCESS);
    pd_ctx[PD_IDX_CLUSTER0].cs_support = false;
    is_deeper_state_ExpectAnyArgsAndReturn(true);
    is_upwards_transition_propagation_ExpectAnyArgsAndReturn(true);
    get_highest_level_from_composite_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUS0CORE0],
        req_params->composite_state,
        MOD_PD_STATE_ON);
    get_level_state_from_composite_state_ExpectAnyArgsAndReturn(0);
    get_level_state_from_composite_state_ExpectAnyArgsAndReturn(1);
    prepare_allow_tree(true, true, true);
    prepare_state_name(MOD_PD_STATE_ON);
    prepare_allow_tree(true, true, true);
    prepare_state_name(MOD_PD_STATE_ON);

    process_power_state_transition_report(
        &pd_ctx[PD_IDX_CLUS0CORE0], report_params);
    TEST_ASSERT_EQUAL(MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUS0CORE0].current_state);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUS0CORE0].requested_state);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUS0CORE0].state_requested_to_driver);
    req_params->composite_state = MOD_PD_STATE_OFF;
}

void test_set_state_simple_core_off(void)
{
    struct fwk_event event;
    struct fwk_event resp_event;
    struct pd_set_state_request *req_params =
        (struct pd_set_state_request *)event.params;
    pd_ctx[PD_IDX_CLUSTER0].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].current_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].cs_support = false;
    pd_ctx[PD_IDX_CLUS0CORE0].requested_state = MOD_PD_STATE_OFF;
    pd_ctx[PD_IDX_CLUS0CORE0].state_requested_to_driver = MOD_PD_STATE_OFF;
    pd_ctx[PD_IDX_CLUS0CORE0].current_state = MOD_PD_STATE_OFF;
    req_params->composite_state = MOD_PD_STATE_OFF;

    is_upwards_transition_propagation_ExpectAnyArgsAndReturn(true);
    get_highest_level_from_composite_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUS0CORE0],
        req_params->composite_state,
        MOD_PD_STATE_ON);
    get_level_state_from_composite_state_ExpectAnyArgsAndReturn(0);
    get_level_state_from_composite_state_ExpectAnyArgsAndReturn(1);

    req_params->composite_state = MOD_PD_STATE_OFF;
    pd_ctx[PD_IDX_CLUS0CORE0].current_state = MOD_PD_STATE_ON;

    fwk_notification_notify_IgnoreAndReturn(FWK_SUCCESS);
    prepare_mocks_for_set_state_request(
        PD_IDX_CLUS0CORE0, MOD_PD_STATE_OFF, false, FWK_SUCCESS);

    process_set_state_request(&pd_ctx[PD_IDX_CLUS0CORE0], &event, &resp_event);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUS0CORE0].requested_state);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUS0CORE0].state_requested_to_driver);
    TEST_ASSERT_EQUAL(MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUS0CORE0].current_state);
}

void test_request_core_cluster_off_while_another_core_is_on(void)
{
    struct fwk_event event;
    struct fwk_event resp_event;
    struct pd_set_state_request *req_params =
        (struct pd_set_state_request *)event.params;
    req_params->composite_state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON, MOD_PD_STATE_OFF, MOD_PD_STATE_OFF);
    /* Modifing CLUSTER0, CLUS0CORE0 and CLUS0CORE1 states to appear ON */
    pd_ctx[PD_IDX_CLUSTER0].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].current_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].driver_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE0].requested_state = MOD_PD_STATE_OFF;
    pd_ctx[PD_IDX_CLUS0CORE0].state_requested_to_driver = MOD_PD_STATE_OFF;
    pd_ctx[PD_IDX_CLUS0CORE0].driver_state = MOD_PD_STATE_OFF;
    pd_ctx[PD_IDX_CLUS0CORE0].current_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE1].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE1].state_requested_to_driver = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE1].current_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE1].driver_state = MOD_PD_STATE_ON;
    fwk_notification_notify_IgnoreAndReturn(FWK_SUCCESS);

    prepare_mocks_for_set_state_request(
        PD_IDX_CLUS0CORE0, MOD_PD_STATE_OFF, false, FWK_SUCCESS);
    is_upwards_transition_propagation_ExpectAnyArgsAndReturn(true);
    get_highest_level_from_composite_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUS0CORE0],
        req_params->composite_state,
        MOD_PD_STATE_ON);
    get_level_state_from_composite_state_ExpectAnyArgsAndReturn(0);
    get_level_state_from_composite_state_ExpectAnyArgsAndReturn(1);
    prepare_allow_tree(true, true, true);
    prepare_allow_tree(true, true, true);
    prepare_state_name(MOD_PD_STATE_ON);

    retrieve_mapped_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUS0CORE0], MOD_PD_STATE_OFF, MOD_PD_STATE_OFF);

    process_set_state_request(&pd_ctx[PD_IDX_CLUS0CORE0], &event, &resp_event);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUS0CORE0].requested_state);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUS0CORE0].state_requested_to_driver);
    /* current state should be on until driver report back */
    TEST_ASSERT_EQUAL(MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUS0CORE0].current_state);
    /* Cluster 0 must not initiate or request transitions as parent (cluster0)
     * must be compatible with all children.
     * i.e. Core 1 is ON which doesn't allow Cluster 0 to be OFF
     */
    TEST_ASSERT_EQUAL(MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUSTER0].requested_state);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver);
    TEST_ASSERT_EQUAL(MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUSTER0].current_state);
}

void test_state_transition_report_core_on_while_transition_init(void)
{
    struct fwk_event report;
    struct pd_power_state_transition_report *report_params =
        (struct pd_power_state_transition_report *)(&report.params);

    /* States after initiated transition and report has not received yet.*/
    pd_ctx[PD_IDX_CLUS0CORE0].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE0].state_requested_to_driver = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE0].current_state = MOD_PD_STATE_OFF;
    pd_ctx[PD_IDX_CLUS0CORE0].driver_state = MOD_PD_STATE_ON;
    report = (struct fwk_event){
        .source_id = pd_ctx[PD_IDX_CLUS0CORE0].driver_id,
        .target_id = pd_ctx[PD_IDX_CLUS0CORE0].id,
        .id = FWK_ID_EVENT(
            FWK_MODULE_IDX_POWER_DOMAIN,
            PD_EVENT_IDX_REPORT_POWER_STATE_TRANSITION),
    };

    report_params->state = MOD_PD_STATE_ON;
    fwk_notification_notify_IgnoreAndReturn(FWK_SUCCESS);
    is_deeper_state_ExpectAnyArgsAndReturn(true);

    process_power_state_transition_report(
        &pd_ctx[PD_IDX_CLUS0CORE0], report_params);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUS0CORE0].requested_state);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUS0CORE0].state_requested_to_driver);
    TEST_ASSERT_EQUAL(MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUS0CORE0].current_state);
}

void test_state_transition_cluster_on_expect_core_transition_init(void)
{
    struct fwk_event report;
    struct pd_power_state_transition_report *report_params =
        (struct pd_power_state_transition_report *)(&report.params);
    /* States after initiated transition and report has not received yet.*/
    pd_ctx[PD_IDX_CLUSTER0].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].current_state = MOD_PD_STATE_OFF;
    pd_ctx[PD_IDX_CLUSTER0].driver_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE0].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE0].state_requested_to_driver = MOD_PD_STATE_OFF;
    pd_ctx[PD_IDX_CLUS0CORE0].current_state = MOD_PD_STATE_OFF;
    pd_ctx[PD_IDX_CLUS0CORE0].driver_state = MOD_PD_STATE_OFF;
    report = (struct fwk_event){
        .source_id = pd_ctx[PD_IDX_CLUSTER0].driver_id,
        .target_id = pd_ctx[PD_IDX_CLUSTER0].id,
        .id = FWK_ID_EVENT(
            FWK_MODULE_IDX_POWER_DOMAIN,
            PD_EVENT_IDX_REPORT_POWER_STATE_TRANSITION),
    };

    report_params->state = MOD_PD_STATE_ON;
    fwk_notification_notify_IgnoreAndReturn(FWK_SUCCESS);
    is_deeper_state_ExpectAnyArgsAndReturn(false);
    is_shallower_state_ExpectAnyArgsAndReturn(true);

    prepare_allow_tree(true, true, true);
    prepare_state_name(MOD_PD_STATE_ON);
    prepare_mocks_for_set_state_request(
        PD_IDX_CLUS0CORE0, MOD_PD_STATE_ON, false, FWK_SUCCESS);

    retrieve_mapped_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUS0CORE0], MOD_PD_STATE_ON, MOD_PD_STATE_ON);

    process_power_state_transition_report(
        &pd_ctx[PD_IDX_CLUSTER0], report_params);
    TEST_ASSERT_EQUAL(MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUSTER0].current_state);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUS0CORE0].requested_state);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUS0CORE0].state_requested_to_driver);
    /* The current state should be off until the driver reports back */
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUS0CORE0].current_state);
}

void test_set_state_on_core_cluster_soc_expect_cluster_transition_init(void)
{
    struct fwk_event event;
    struct fwk_event resp_event;
    struct pd_set_state_request *req_params =
        (struct pd_set_state_request *)event.params;

    req_params->composite_state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON, MOD_PD_STATE_ON, MOD_PD_STATE_ON);
    fwk_notification_notify_IgnoreAndReturn(FWK_SUCCESS);

    prepare_mocks_for_set_state_request(
        PD_IDX_CLUSTER0, MOD_PD_STATE_ON, false, FWK_SUCCESS);

    is_upwards_transition_propagation_ExpectAnyArgsAndReturn(false);
    get_highest_level_from_composite_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUS0CORE0],
        req_params->composite_state,
        MOD_PD_STATE_ON);
    get_level_state_from_composite_state_ExpectAnyArgsAndReturn(1);
    get_level_state_from_composite_state_ExpectAnyArgsAndReturn(1);
    prepare_allow_tree(true, true, true);
    prepare_allow_tree(true, true, true);
    prepare_state_name(MOD_PD_STATE_ON);

    retrieve_mapped_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUSTER0], MOD_PD_STATE_ON, MOD_PD_STATE_ON);
    retrieve_mapped_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUS0CORE0], MOD_PD_STATE_ON, MOD_PD_STATE_ON);

    process_set_state_request(&pd_ctx[PD_IDX_CLUS0CORE0], &event, &resp_event);
    TEST_ASSERT_EQUAL(MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUSTER0].requested_state);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver);
    /* The current state should be off until the driver reports back */
    TEST_ASSERT_EQUAL(MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUSTER0].current_state);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUS0CORE0].requested_state);
    /*
     *  Transition of CLUS0CORE0 is not initiated,
     *  it should wait for CLUSTER0 to be on.
     */
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUS0CORE0].state_requested_to_driver);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_OFF, pd_ctx[PD_IDX_CLUS0CORE0].current_state);
}

void test_state_transition_report_cluster_on_while_transition_init(void)
{
    struct fwk_event report;
    struct pd_power_state_transition_report *report_params =
        (struct pd_power_state_transition_report *)(&report.params);
    /* States after initiated transition and report has not received yet.*/
    pd_ctx[PD_IDX_CLUSTER0].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].current_state = MOD_PD_STATE_OFF;
    pd_ctx[PD_IDX_CLUSTER0].driver_state = MOD_PD_STATE_ON;
    report = (struct fwk_event){
        .source_id = pd_ctx[PD_IDX_CLUSTER0].driver_id,
        .target_id = pd_ctx[PD_IDX_CLUSTER0].id,
        .id = FWK_ID_EVENT(
            FWK_MODULE_IDX_POWER_DOMAIN,
            PD_EVENT_IDX_REPORT_POWER_STATE_TRANSITION),
    };
    report_params->state = MOD_PD_STATE_ON;
    fwk_notification_notify_IgnoreAndReturn(FWK_SUCCESS);
    is_deeper_state_ExpectAnyArgsAndReturn(true);

    process_power_state_transition_report(
        &pd_ctx[PD_IDX_CLUSTER0], report_params);
    TEST_ASSERT_EQUAL(MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUSTER0].requested_state);
    TEST_ASSERT_EQUAL(
        MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver);
    TEST_ASSERT_EQUAL(MOD_PD_STATE_ON, pd_ctx[PD_IDX_CLUSTER0].current_state);
}

void test_initiate_power_state_transition_not_core(void)
{
    int status;

    pd_ctx[PD_IDX_CLUSTER0].requested_state = MOD_PD_STATE_ON;
    prepare_mocks_for_set_state_request(
        PD_IDX_CLUSTER0, MOD_PD_STATE_ON, false, FWK_SUCCESS);
    prepare_state_name(MOD_PD_STATE_ON);

    retrieve_mapped_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUSTER0], MOD_PD_STATE_ON, MOD_PD_STATE_ON);

    status = initiate_power_state_transition(&pd_ctx[PD_IDX_CLUSTER0]);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(
        pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver, MOD_PD_STATE_ON);
}

void test_initiate_power_state_transition_core_original_state(void)
{
    int status;

    pd_ctx[PD_IDX_CLUSTER0].requested_state = MOD_PD_STATE_ON;

    prepare_mocks_for_set_state_request(
        PD_IDX_CLUSTER0, MOD_PD_STATE_ON, false, FWK_SUCCESS);
    prepare_state_name(MOD_PD_STATE_ON);

    retrieve_mapped_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUSTER0], MOD_PD_STATE_ON, MOD_PD_STATE_ON);

    status = initiate_power_state_transition(&pd_ctx[PD_IDX_CLUSTER0]);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(
        pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver, MOD_PD_STATE_ON);
}

void test_initiate_power_state_transition_core_new_state(void)
{
    int status;
    unsigned int new_state;

    pd_ctx[PD_IDX_CLUS0CORE0].requested_state = MOD_PD_STATE_OFF_0;

    new_state = pd_ctx[PD_IDX_CLUS0CORE0]
                    .config->pd_state_mapping_table[MOD_PD_STATE_OFF_0];

    fwk_module_get_element_name_ExpectAndReturn(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, PD_IDX_CLUS0CORE0),
        pd_element_table[PD_IDX_CLUS0CORE0].name);

    pd_driver_deny_ExpectAndReturn(
        pd_ctx[PD_IDX_CLUS0CORE0].driver_id, MOD_PD_STATE_OFF_0, false);

    pd_driver_set_state_ExpectAndReturn(
        pd_ctx[PD_IDX_CLUS0CORE0].driver_id, new_state, FWK_SUCCESS);

    prepare_state_name(MOD_PD_STATE_OFF_0);

    retrieve_mapped_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUS0CORE0], MOD_PD_STATE_OFF_0, MOD_PD_STATE_SLEEP);
    retrieve_mapped_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUS0CORE0], MOD_PD_STATE_OFF_0, MOD_PD_STATE_SLEEP);

    status = initiate_power_state_transition(&pd_ctx[PD_IDX_CLUS0CORE0]);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(
        pd_ctx[PD_IDX_CLUS0CORE0].state_requested_to_driver,
        MOD_PD_STATE_OFF_0);
}

void test_pd_get_domain_type(void)
{
    int status;
    fwk_id_t pd_id;
    enum mod_pd_type type;

    fwk_module_is_valid_element_id_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(PD_IDX_CLUS0CORE0);

    status = pd_get_domain_type(pd_id, &type);

    TEST_ASSERT_EQUAL(MOD_PD_TYPE_CORE, type);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void test_pd_get_domain_parent_id(void)
{
    int status;
    fwk_id_t pd_id;
    fwk_id_t parent_pd_id;
    fwk_id_t local_id = pd_ctx[PD_IDX_CLUS0CORE0].parent->id;

    fwk_module_is_valid_element_id_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(PD_IDX_CLUS0CORE0);

    status = pd_get_domain_parent_id(pd_id, &parent_pd_id);

    TEST_ASSERT_EQUAL_MEMORY(&local_id, &parent_pd_id, sizeof(fwk_id_t));
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void test_pd_get_domain_parent_id_null_param(void)
{
    int status;
    fwk_id_t pd_id;
    status = pd_get_domain_parent_id(pd_id, NULL);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void test_process_get_state_request_no_cs(void)
{
    unsigned int state;
    pd_ctx[PD_IDX_CLUS0CORE0].cs_support = false;

    process_get_state_request(&pd_ctx[PD_IDX_CLUS0CORE0], &state);

    TEST_ASSERT_EQUAL(MOD_PD_STATE_OFF, state);
}

void test_process_get_state_request_cs(void)
{
    unsigned int state;

    pd_ctx[PD_IDX_CLUS0CORE0].cs_support = true;
    pd_ctx[PD_IDX_CLUS0CORE0].composite_state_levels_mask = MOD_PD_CS_STATE_MASK
        << MOD_PD_CS_LEVEL_SHIFT;

    for (int i = 0; i < PD_IDX_COUNT; ++i) {
        pd_ctx[i].current_state = MOD_PD_STATE_ON;
    }

    uint32_t composite_state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON, MOD_PD_STATE_ON, MOD_PD_STATE_ON);

    /* For the composite table mask size */
    number_of_bits_to_shift_ExpectAndReturn(
        pd_ctx[PD_IDX_CLUS0CORE0].composite_state_mask_table[0], 0);
    number_of_bits_to_shift_ExpectAndReturn(
        pd_ctx[PD_IDX_CLUS0CORE0].composite_state_mask_table[1], 4);
    number_of_bits_to_shift_ExpectAndReturn(
        pd_ctx[PD_IDX_CLUS0CORE0].composite_state_mask_table[2], 8);

    /* For the level shift */
    number_of_bits_to_shift_ExpectAndReturn(
        pd_ctx[PD_IDX_CLUS0CORE0].composite_state_levels_mask, 16);

    process_get_state_request(&pd_ctx[PD_IDX_CLUS0CORE0], &state);

    TEST_ASSERT_EQUAL(composite_state, state);
}

void test_initiate_power_state_transition(void)
{
    int status;

    pd_ctx[PD_IDX_CLUSTER0].requested_state = MOD_PD_STATE_ON;

    prepare_mocks_for_set_state_request(
        PD_IDX_CLUSTER0, MOD_PD_STATE_ON, false, FWK_SUCCESS);
    prepare_state_name(MOD_PD_STATE_ON);

    retrieve_mapped_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUSTER0], MOD_PD_STATE_ON, MOD_PD_STATE_ON);

    status = initiate_power_state_transition(&pd_ctx[PD_IDX_CLUSTER0]);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(
        pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver, MOD_PD_STATE_ON);
}

void test_initiate_power_state_transition_fail_param(void)
{
    int status;

    pd_ctx[PD_IDX_CLUSTER0].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver = MOD_PD_STATE_OFF;

    prepare_mocks_for_set_state_request(
        PD_IDX_CLUSTER0, MOD_PD_STATE_ON, false, FWK_E_PARAM);
    prepare_state_name(MOD_PD_STATE_ON);

    retrieve_mapped_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUSTER0], MOD_PD_STATE_ON, MOD_PD_STATE_ON);

    status = initiate_power_state_transition(&pd_ctx[PD_IDX_CLUSTER0]);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
    TEST_ASSERT_EQUAL(
        pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver, MOD_PD_STATE_OFF);
}

void test_process_power_state_transition_report_deeper_state(void)
{
    pd_ctx[PD_IDX_CLUSTER0].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].driver_state = MOD_PD_STATE_ON;

    is_allowed_by_parent_and_children_ExpectAnyArgsAndReturn(true);
    initiate_power_state_pre_transition_notification_ExpectAnyArgsAndReturn(
        true);

    prepare_mocks_for_set_state_request(
        PD_IDX_CLUSTER0, MOD_PD_STATE_ON, false, FWK_SUCCESS);
    prepare_state_name(MOD_PD_STATE_ON);

    retrieve_mapped_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUSTER0], MOD_PD_STATE_ON, MOD_PD_STATE_ON);

    process_power_state_transition_report_deeper_state(
        &pd_ctx[PD_IDX_CLUS0CORE0]);
    TEST_ASSERT_EQUAL(
        pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver, MOD_PD_STATE_ON);
}

void test_process_power_state_transition_report_shallower_state(void)
{
    pd_ctx[PD_IDX_CLUS0CORE0].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE0].state_requested_to_driver = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE0].driver_state = MOD_PD_STATE_ON;

    is_allowed_by_parent_and_children_ExpectAnyArgsAndReturn(true);
    initiate_power_state_pre_transition_notification_ExpectAnyArgsAndReturn(
        true);

    prepare_mocks_for_set_state_request(
        PD_IDX_CLUS0CORE0, MOD_PD_STATE_ON, false, FWK_SUCCESS);
    prepare_state_name(MOD_PD_STATE_ON);

    retrieve_mapped_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUS0CORE0], MOD_PD_STATE_ON, MOD_PD_STATE_ON);

    process_power_state_transition_report_shallower_state(
        &pd_ctx[PD_IDX_CLUSTER0]);
    TEST_ASSERT_EQUAL(
        pd_ctx[PD_IDX_CLUS0CORE0].state_requested_to_driver, MOD_PD_STATE_ON);
}

void test_complete_system_suspend_no_cs(void)
{
    int status;
    pd_ctx[PD_IDX_CLUS0CORE0].cs_support = false;

    status = complete_system_suspend(&pd_ctx[PD_IDX_CLUS0CORE0]);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_complete_system_suspend(void)
{
    int status;
    uint32_t composite_state = 0;

    pd_ctx[PD_IDX_CLUS0CORE0].cs_support = true;

    /* For the composite table mask size */
    number_of_bits_to_shift_ExpectAndReturn(
        pd_ctx[PD_IDX_CLUS0CORE0].composite_state_mask_table[0], 0);
    number_of_bits_to_shift_ExpectAndReturn(
        pd_ctx[PD_IDX_CLUS0CORE0].composite_state_mask_table[1], 4);
    number_of_bits_to_shift_ExpectAndReturn(
        pd_ctx[PD_IDX_CLUS0CORE0].composite_state_mask_table[2], 8);

    composite_state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_2,
        0,
        MOD_PD_STATE_OFF,
        MOD_PD_STATE_OFF,
        MOD_PD_STATE_OFF);

    is_upwards_transition_propagation_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUS0CORE0], composite_state, true);

    get_highest_level_from_composite_state_ExpectAndReturn(
        &pd_ctx[PD_IDX_CLUS0CORE0], composite_state, MOD_PD_STATE_OFF);

    get_level_state_from_composite_state_ExpectAnyArgsAndReturn(0);

    status = complete_system_suspend(&pd_ctx[PD_IDX_CLUS0CORE0]);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

static bool is_in_transiton_mock(
    struct pd_ctx *pd,
    unsigned int state,
    int call_no)
{
    return pd->current_state != state;
}

void test_system_suspend_multiple_active_cores(void)
{
    int status;

    __fwk_put_event_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    is_state_in_transition_StubWithCallback(is_in_transiton_mock);

    pd_ctx[PD_IDX_CLUSTER0].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].current_state = MOD_PD_STATE_ON;

    pd_ctx[PD_IDX_CLUS0CORE0].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE0].state_requested_to_driver = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE0].current_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE1].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE1].state_requested_to_driver = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE1].current_state = MOD_PD_STATE_ON;

    status = pd_system_suspend(MOD_SYSTEM_POWER_POWER_STATE_SLEEP0);

    TEST_ASSERT_EQUAL(status, FWK_E_STATE);
}

void test_system_suspend_single_active_core(void)
{
    int status;

    __fwk_put_event_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    is_state_in_transition_StubWithCallback(is_in_transiton_mock);

    pd_ctx[PD_IDX_CLUSTER0].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].state_requested_to_driver = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUSTER0].current_state = MOD_PD_STATE_ON;

    pd_ctx[PD_IDX_CLUS0CORE0].requested_state = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE0].state_requested_to_driver = MOD_PD_STATE_ON;
    pd_ctx[PD_IDX_CLUS0CORE0].current_state = MOD_PD_STATE_ON;

    status = pd_system_suspend(MOD_SYSTEM_POWER_POWER_STATE_SLEEP0);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

int power_domain_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_set_state_cluster_on_expect_transition_init);
    RUN_TEST(test_set_state_core_on_while_cluster_off_expect_error_in_resp);
    RUN_TEST(test_set_state_error_in_initiating_transition);
    RUN_TEST(test_set_state_failed_transition_while_driver_set);
    RUN_TEST(test_set_state_simple_core_wakup);
    RUN_TEST(test_set_state_simple_core_off);
    RUN_TEST(test_request_core_cluster_off_while_another_core_is_on);
    RUN_TEST(test_state_transition_report_core_on_while_transition_init);
    RUN_TEST(test_state_transition_cluster_on_expect_core_transition_init);
    RUN_TEST(test_set_state_on_core_cluster_soc_expect_cluster_transition_init);
    RUN_TEST(test_state_transition_report_cluster_on_while_transition_init);
    RUN_TEST(test_initiate_power_state_transition_not_core);
    RUN_TEST(test_initiate_power_state_transition_core_new_state);
    RUN_TEST(test_initiate_power_state_transition_core_original_state);
    RUN_TEST(test_pd_get_domain_type);
    RUN_TEST(test_pd_get_domain_parent_id);
    RUN_TEST(test_pd_get_domain_parent_id_null_param);
    RUN_TEST(test_process_get_state_request_no_cs);
    RUN_TEST(test_process_get_state_request_cs);
    RUN_TEST(test_process_power_state_transition_report_deeper_state);
    RUN_TEST(test_process_power_state_transition_report_shallower_state);
    RUN_TEST(test_initiate_power_state_transition);
    RUN_TEST(test_initiate_power_state_transition_fail_param);
    RUN_TEST(test_complete_system_suspend_no_cs);
    RUN_TEST(test_complete_system_suspend);
    RUN_TEST(test_system_suspend_multiple_active_cores);
    RUN_TEST(test_system_suspend_single_active_core);
    return UNITY_END();
}

int main(void)
{
    return power_domain_test_main();
}
