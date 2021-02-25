/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <internal/fwk_module.h>

#include <fwk_assert.h>
#include <fwk_macros.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_test.h>

#include <stdlib.h>
#include <string.h>

#define ELEM0_IDX          0
#define ELEM1_IDX          1
#define ELEM2_IDX          0
#define SUB_ELEM0_IDX      0
#define API0_IDX           0
#define API1_IDX           1
#define EVENT0_IDX         0
#define EVENT1_IDX         1
#define EVENT2_IDX         2
#define NOTIFICATION0_IDX  0
#define NOTIFICATION1_IDX  1
#define NOTIFICATION2_IDX  2

#define ELEM0_ID FWK_ID_ELEMENT(FWK_MODULE_IDX_FAKE0, ELEM0_IDX)
#define ELEM1_ID FWK_ID_ELEMENT(FWK_MODULE_IDX_FAKE0, ELEM1_IDX)
#define ELEM2_ID FWK_ID_ELEMENT(FWK_MODULE_IDX_FAKE1, ELEM2_IDX)
#define SUB_ELEM0_ID \
    FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_FAKE0, ELEM0_IDX, SUB_ELEM0_IDX)
#define API0_ID FWK_ID_API(FWK_MODULE_IDX_FAKE0, API0_IDX)
#define API1_ID FWK_ID_API(FWK_MODULE_IDX_FAKE0, API1_IDX)
#define EVENT0_ID FWK_ID_EVENT(FWK_MODULE_IDX_FAKE1, EVENT0_IDX)
#define EVENT1_ID FWK_ID_EVENT(FWK_MODULE_IDX_FAKE1, EVENT1_IDX)
#define EVENT2_ID FWK_ID_EVENT(FWK_MODULE_IDX_FAKE1, EVENT2_IDX)
#define NOTIFICATION0_ID \
    FWK_ID_NOTIFICATION(FWK_MODULE_IDX_FAKE0, NOTIFICATION0_IDX)
#define NOTIFICATION1_ID \
    FWK_ID_NOTIFICATION(FWK_MODULE_IDX_FAKE0, NOTIFICATION1_IDX)
#define NOTIFICATION2_ID \
    FWK_ID_NOTIFICATION(FWK_MODULE_IDX_FAKE0, NOTIFICATION2_IDX)

/* Fake API */
struct fake_api {
    int (*init)(fwk_id_t module_id, unsigned int element_count,
                const void *data);
    int (*element_init)(fwk_id_t element_id, unsigned int sub_element_count,
                        const void *data);
};

/* Fake element data */
struct config_element_data {
    int fake_val;
    fwk_id_t ref;
};

static struct config_element_data config_elem0;
static struct config_element_data config_elem1;
static struct config_element_data config_elem2;

static struct fwk_element fake_element_desc_table0[3];
static struct fwk_element fake_element_desc_table1[2];

/* Fake module data */
struct config_module_data {
    int fake_val;
};

static struct config_module_data config_module0;
static struct config_module_data config_module1;

extern struct fwk_module *module_table[FWK_MODULE_IDX_COUNT];
extern struct fwk_module_config *module_config_table[FWK_MODULE_IDX_COUNT];

static struct fwk_module fake_module_desc0;
static struct fwk_module fake_module_desc1;

static struct fwk_module_config fake_module_config0;
static struct fwk_module_config fake_module_config1;

/* Mock functions */
static int init_return_val;
static int element_init_return_val;
static int post_init_return_val;
static int bind_return_val;
static int bind_count_call;
static int start_return_val;
static int start_count_call;
static int process_bind_request_return_val;
static bool process_bind_request_return_api;
static bool get_element_table0_return_val;
static bool get_element_table1_return_val;
static int process_event_return_val;
static int init_return_val;

static int init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    (void) module_id;
    (void) element_count;
    (void) data;
    return init_return_val;
}

static int element_init(fwk_id_t element_id, unsigned int sub_element_count,
    const void *data)
{
    (void) element_id;
    (void) data;
    return element_init_return_val;
}

static int post_init(fwk_id_t module_id)
{
    (void) module_id;
    return post_init_return_val;
}

static int bind(fwk_id_t id, unsigned int round_number)
{
    (void) id;
    (void) round_number;
    bind_count_call++;
    return bind_return_val;
}

static int start(fwk_id_t id)
{
    (void) id;
    start_count_call++;
    return start_return_val;
}

static struct fake_api fake_api = {
    .init = init,
    .element_init = element_init
};

static int process_bind_request(fwk_id_t source_id, fwk_id_t target_id,
    fwk_id_t api_id, const void **api)
{
    (void) source_id;
    (void) target_id;
    (void) api_id;
    if (process_bind_request_return_api)
        *api = &fake_api;

    return process_bind_request_return_val;
}

static const struct fwk_element *get_element_table0(fwk_id_t module_id)
{
    (void) module_id;
    if (get_element_table0_return_val)
        return fake_element_desc_table0;
    else
        return NULL;
}

static const struct fwk_element *get_element_table1(fwk_id_t module_id)
{
    (void) module_id;
    if (get_element_table1_return_val)
        return fake_element_desc_table1;
    else
        return NULL;
}

/* Wrapped functions */

/*
 * Allow to choose the behavior of the wrapped function fwk_mm_calloc.
 * If 0, fwk_mm_calloc(num, size) returns calloc(num, size).
 * If 1, fwk_mm_calloc(num, size) return calloc(num, size) the first time it
 * is called and NULL the following times.
 * If 2, fwk_mm_calloc(num, size) returns NULL.
 */
static int fwk_mm_calloc_return;
void * __wrap_fwk_mm_calloc(size_t num, size_t size)
{
    if (num == 0)
        return NULL;
    if (fwk_mm_calloc_return == 0)
        return calloc(num, size);
    else if (fwk_mm_calloc_return <= 4) {
        fwk_mm_calloc_return++;
        return calloc(num, size);
    }
    return NULL;
}

int __wrap___fwk_init(size_t event_count)
{
    (void) event_count;
    return init_return_val;
}

void __wrap___fwk_run_main_loop(void)
{
}

int __wrap___fwk_notification_init(size_t notification_count)
{
    (void) notification_count;
    return FWK_SUCCESS;
}

static void test_case_setup(void)
{
    /* Default values */
    get_element_table0_return_val = true;
    get_element_table1_return_val = true;
    fwk_mm_calloc_return = 0;
    init_return_val = FWK_SUCCESS;
    element_init_return_val = FWK_SUCCESS;
    post_init_return_val = FWK_SUCCESS;
    bind_return_val = FWK_SUCCESS;
    start_return_val = FWK_SUCCESS;
    process_bind_request_return_val = FWK_SUCCESS;
    process_bind_request_return_api = true;
    process_event_return_val = FWK_SUCCESS;
    init_return_val = FWK_SUCCESS;

    bind_count_call = 0;
    start_count_call = 0;

    config_elem0.fake_val = 5;
    config_elem0.ref = fwk_id_build_element_id(fwk_module_id_fake0, ELEM0_IDX);

    config_elem1.fake_val = 6;
    config_elem1.ref = fwk_id_build_element_id(fwk_module_id_fake0, ELEM1_IDX);

    config_elem2.fake_val = 7;
    config_elem2.ref = fwk_id_build_element_id(fwk_module_id_fake1, ELEM2_IDX);

    config_module0.fake_val = 8;
    config_module1.fake_val = 9;

    fake_module_desc0.api_count = 2;
    fake_module_desc0.event_count = 0;
    fake_module_desc0.notification_count = 3;
    fake_module_desc0.type = FWK_MODULE_TYPE_DRIVER;
    fake_module_desc0.init = init;
    fake_module_desc0.element_init = element_init;
    fake_module_desc0.post_init = post_init;
    fake_module_desc0.bind = bind;
    fake_module_desc0.start = start;
    fake_module_desc0.process_bind_request = process_bind_request;

    fake_module_desc1.api_count = 0;
    fake_module_desc1.event_count = 3;
    fake_module_desc1.notification_count = 0;
    fake_module_desc1.type = FWK_MODULE_TYPE_DRIVER;
    fake_module_desc1.init = init;
    fake_module_desc1.element_init = element_init;
    fake_module_desc1.post_init = post_init;
    fake_module_desc1.bind = bind;
    fake_module_desc1.start = start;

    fake_element_desc_table0[0].name = "FAKE ELEM 0";
    fake_element_desc_table0[0].data = &config_elem0;
    fake_element_desc_table0[0].sub_element_count = 1;
    fake_element_desc_table0[1].name = "FAKE ELEM 1";
    fake_element_desc_table0[1].data = &config_elem1;
    fake_element_desc_table0[2].name = NULL;
    fake_element_desc_table0[2].data = NULL;

    fake_element_desc_table1[0].name = "FAKE ELEM 2";
    fake_element_desc_table1[0].data = &config_elem2;
    fake_element_desc_table1[1].name = NULL;
    fake_element_desc_table1[1].data = NULL;

    fake_module_config0.elements.type = FWK_MODULE_ELEMENTS_TYPE_DYNAMIC;
    fake_module_config0.elements.generator = get_element_table0;
    fake_module_config0.data = &config_module0;

    fake_module_config1.elements.type = FWK_MODULE_ELEMENTS_TYPE_DYNAMIC;
    fake_module_config1.elements.generator = get_element_table1;
    fake_module_config1.data = &config_module1;

    module_table[0] = &fake_module_desc0;
    module_table[1] = &fake_module_desc1;
    module_table[2] = NULL;

    module_config_table[0] = &fake_module_config0;
    module_config_table[1] = &fake_module_config1;
    module_config_table[2] = NULL;

    fwk_module_reset();
    fwk_module_start();
}

static void test_fwk_module_is_valid_module_id(void)
{
    fwk_id_t id;
    bool result;

    /* Valid module ID */
    id = fwk_module_id_fake0;
    result = fwk_module_is_valid_module_id(id);
    assert(result);

    /* Valid module ID */
    id = fwk_module_id_fake1;
    result = fwk_module_is_valid_module_id(id);
    assert(result);

    /* ID larger than the number of modules */
    id = FWK_ID_MODULE(0x3);
    result = fwk_module_is_valid_module_id(id);
    assert(!result);

    /* Invalid type */
    id = ELEM0_ID;
    result = fwk_module_is_valid_module_id(id);
    assert(!result);
}

static void test_fwk_module_is_valid_event_id(void)
{
    fwk_id_t id;
    bool result;

    /* Valid event ID */
    id = EVENT0_ID;
    result = fwk_module_is_valid_event_id(id);
    assert(result);

    /* Valid event ID */
    id = EVENT2_ID;
    result = fwk_module_is_valid_event_id(id);
    assert(result);

    /* Module IDX non valid */
    id = FWK_ID_EVENT(0x03, 0x00);
    result = fwk_module_is_valid_event_id(id);
    assert(!result);

    /* Event IDX non valid */
    id = FWK_ID_EVENT(FWK_MODULE_IDX_FAKE1, 0x03);
    result = fwk_module_is_valid_event_id(id);
    assert(!result);

    /* Module IDX non valid */
    id = FWK_ID_EVENT(FWK_MODULE_IDX_FAKE0, 0x00);
    result = fwk_module_is_valid_event_id(id);
    assert(!result);

    /* Invalid type */
    id = fwk_module_id_fake0;
    result = fwk_module_is_valid_event_id(id);
    assert(!result);
}

static void test_fwk_module_is_valid_notification_id(void)
{
    fwk_id_t id;
    bool result;

    /* Valid notification ID */
    id = NOTIFICATION0_ID;
    result = fwk_module_is_valid_notification_id(id);
    assert(result);

    /* Valid notification ID */
    id = NOTIFICATION2_ID;
    result = fwk_module_is_valid_notification_id(id);
    assert(result);

    /* Module IDX non valid */
    id = FWK_ID_NOTIFICATION(0x03, 0x00);
    result = fwk_module_is_valid_notification_id(id);
    assert(!result);

    /* Event IDX non valid */
    id = FWK_ID_NOTIFICATION(FWK_MODULE_IDX_FAKE0, 0x03);
    result = fwk_module_is_valid_notification_id(id);
    assert(!result);

    /* Module IDX non valid */
    id = FWK_ID_NOTIFICATION(FWK_MODULE_IDX_FAKE1, 0x00);
    result = fwk_module_is_valid_notification_id(id);
    assert(!result);

    /* Invalid type */
    id = fwk_module_id_fake0;
    result = fwk_module_is_valid_notification_id(id);
    assert(!result);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_fwk_module_is_valid_module_id),
    FWK_TEST_CASE(test_fwk_module_is_valid_event_id),
    FWK_TEST_CASE(test_fwk_module_is_valid_notification_id),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_module",
    .test_case_setup = test_case_setup,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
