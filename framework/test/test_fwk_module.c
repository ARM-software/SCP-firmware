/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <internal/fwk_module.h>

#include <fwk_assert.h>
#include <fwk_macros.h>
#include <fwk_status.h>
#include <fwk_test.h>

#include <stdlib.h>
#include <string.h>

#define MODULE0_IDX        0
#define MODULE1_IDX        1
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

#define MODULE0_ID        FWK_ID_MODULE(MODULE0_IDX)
#define MODULE1_ID        FWK_ID_MODULE(MODULE1_IDX)
#define ELEM0_ID          FWK_ID_ELEMENT(MODULE0_IDX, ELEM0_IDX)
#define ELEM1_ID          FWK_ID_ELEMENT(MODULE0_IDX, ELEM1_IDX)
#define ELEM2_ID          FWK_ID_ELEMENT(MODULE1_IDX, ELEM2_IDX)
#define SUB_ELEM0_ID      FWK_ID_SUB_ELEMENT(MODULE0_IDX, \
                                             ELEM0_IDX, \
                                             SUB_ELEM0_IDX)
#define API0_ID           FWK_ID_API(MODULE0_IDX, API0_IDX)
#define API1_ID           FWK_ID_API(MODULE0_IDX, API1_IDX)
#define EVENT0_ID         FWK_ID_EVENT(MODULE1_IDX, EVENT0_IDX)
#define EVENT1_ID         FWK_ID_EVENT(MODULE1_IDX, EVENT1_IDX)
#define EVENT2_ID         FWK_ID_EVENT(MODULE1_IDX, EVENT2_IDX)
#define NOTIFICATION0_ID  FWK_ID_NOTIFICATION(MODULE0_IDX, NOTIFICATION0_IDX)
#define NOTIFICATION1_ID  FWK_ID_NOTIFICATION(MODULE0_IDX, NOTIFICATION1_IDX)
#define NOTIFICATION2_ID  FWK_ID_NOTIFICATION(MODULE0_IDX, NOTIFICATION2_IDX)

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

struct fwk_module *module_table[3];
struct fwk_module_config *module_config_table[3];

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
static int thread_init_return_val;

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

int __wrap___fwk_thread_init(size_t event_count)
{
    (void) event_count;
    return thread_init_return_val;
}

void __wrap___fwk_thread_run(void)
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
    thread_init_return_val = FWK_SUCCESS;

    bind_count_call = 0;
    start_count_call = 0;

    config_elem0.fake_val = 5;
    config_elem0.ref = fwk_id_build_element_id(MODULE0_ID, ELEM0_IDX);

    config_elem1.fake_val = 6;
    config_elem1.ref = fwk_id_build_element_id(MODULE0_ID, ELEM1_IDX);

    config_elem2.fake_val = 7;
    config_elem2.ref = fwk_id_build_element_id(MODULE1_ID, ELEM2_IDX);

    config_module0.fake_val = 8;
    config_module1.fake_val = 9;

    fake_module_desc0.name = "FAKE MODULE 0";
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

    fake_module_desc1.name = "FAKE MODULE 1";
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

    fake_module_config0.get_element_table = get_element_table0;
    fake_module_config0.data = &config_module0;

    fake_module_config1.get_element_table = get_element_table1;
    fake_module_config1.data = &config_module1;

    module_table[0] = &fake_module_desc0;
    module_table[1] = &fake_module_desc1;
    module_table[2] = NULL;

    module_config_table[0] = &fake_module_config0;
    module_config_table[1] = &fake_module_config1;
    module_config_table[2] = NULL;

    __fwk_module_reset();
    __fwk_module_init();
}

static void test___fwk_module_init_module_desc_bad_params(void)
{
    int result;

    /* Module descriptor has bad name */
    fake_module_desc0.name = NULL;
    __fwk_module_reset();
    result = __fwk_module_init();
    assert(result == FWK_E_PARAM);
    fake_module_desc0.name = "FAKE MODULE 0";

    /* Module descriptor has bad type */
    fake_module_desc0.type = FWK_MODULE_TYPE_COUNT;
    __fwk_module_reset();
    result = __fwk_module_init();
    assert(result == FWK_E_PARAM);
    fake_module_desc0.type = FWK_MODULE_TYPE_DRIVER;

    /* Module descriptor has no init function */
    fake_module_desc0.init = NULL;
    __fwk_module_reset();
    result = __fwk_module_init();
    assert(result == FWK_E_PARAM);
    fake_module_desc0.init = init;

    /* Module descriptor has no element_init function */
    fake_module_desc0.element_init = NULL;
    __fwk_module_reset();
    result = __fwk_module_init();
    assert(result == FWK_E_PARAM);
    fake_module_desc0.element_init = element_init;

    /* Module 0 has no config*/
    module_config_table[0] = NULL;
    assert(result == FWK_E_PARAM);
    module_config_table[0] = &fake_module_config0;

    /* Element description table of module 0 is NULL */
    get_element_table0_return_val = false;
    __fwk_module_reset();
    result = __fwk_module_init();
    assert(result == FWK_E_PARAM);
    get_element_table0_return_val = true;
}

static void test___fwk_module_init_failure(void)
{
    int result;
    enum fwk_module_state state;

    init_return_val = FWK_E_HANDLER;
    element_init_return_val = FWK_E_HANDLER;
    post_init_return_val = FWK_E_HANDLER;
    bind_return_val = FWK_E_HANDLER;
    start_return_val = FWK_E_HANDLER;
    process_bind_request_return_val = FWK_E_HANDLER;

    /* Init function failure */
    init_return_val = FWK_E_PARAM;
    __fwk_module_reset();
    result = __fwk_module_init();
    assert(result == FWK_E_PARAM);
    init_return_val = FWK_SUCCESS;

    result = __fwk_module_get_state(MODULE0_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_UNINITIALIZED);

    /* Element_init function failure */
    element_init_return_val = FWK_E_PARAM;
    __fwk_module_reset();
    result = __fwk_module_init();
    assert(result == FWK_E_PARAM);

    result = __fwk_module_get_state(MODULE0_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_UNINITIALIZED);

    result = __fwk_module_get_state(MODULE1_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_UNINITIALIZED);

    element_init_return_val = FWK_SUCCESS;

    /* post_init function failure */
    post_init_return_val = FWK_E_PARAM;
    __fwk_module_reset();
    result = __fwk_module_init();
    assert(result == FWK_E_PARAM);
    post_init_return_val = FWK_SUCCESS;

    result = __fwk_module_get_state(ELEM0_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_INITIALIZED);

    result = __fwk_module_get_state(ELEM1_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_INITIALIZED);

    result = __fwk_module_get_state(MODULE0_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_UNINITIALIZED);

    result = __fwk_module_get_state(MODULE1_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_UNINITIALIZED);
}

static void test___fwk_module_init_bind_failure(void)
{
    int result;
    enum fwk_module_state state;

    bind_return_val = FWK_E_HANDLER;
    start_return_val = FWK_E_HANDLER;
    process_bind_request_return_val = FWK_E_HANDLER;

    /* Bind function failure */
    bind_return_val = FWK_E_PARAM;
    __fwk_module_reset();
    result = __fwk_module_init();
    assert(result == FWK_E_PARAM);
    bind_return_val = FWK_SUCCESS;

    result = __fwk_module_get_state(ELEM0_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_INITIALIZED);

    result = __fwk_module_get_state(ELEM1_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_INITIALIZED);

    result = __fwk_module_get_state(MODULE0_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_INITIALIZED);

    result = __fwk_module_get_state(MODULE1_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_INITIALIZED);

}

static void test___fwk_module_init_start_failure(void)
{
    int result;
    enum fwk_module_state state;

    start_return_val = FWK_E_HANDLER;
    process_bind_request_return_val = FWK_E_HANDLER;

    /* Start function failure */
    start_return_val = FWK_E_PARAM;
    __fwk_module_reset();
    result = __fwk_module_init();
    assert(result == FWK_E_PARAM);
    start_return_val = FWK_SUCCESS;

    result = __fwk_module_get_state(ELEM0_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_BOUND);

    result = __fwk_module_get_state(ELEM1_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_BOUND);

    result = __fwk_module_get_state(MODULE0_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_BOUND);

    result = __fwk_module_get_state(MODULE1_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == (FWK_MODULE_STATE_BOUND));
}

static void test_fwk_thread_failure(void)
{
    int result;

    /* fwk_thread_init failed */
    thread_init_return_val = FWK_E_PARAM;
    __fwk_module_reset();
    result = __fwk_module_init();
    assert(result == FWK_E_PARAM);
    thread_init_return_val = FWK_SUCCESS;
}

static void check_correct_initialization(void)
{
    int result;
    enum fwk_module_state state;

    /* Check module 0 is correctly initialized */
    result = __fwk_module_get_state(MODULE0_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_STARTED);
    assert(__fwk_module_get_ctx(MODULE0_ID)->desc == &fake_module_desc0);
    assert(__fwk_module_get_ctx(MODULE0_ID)->config == &fake_module_config0);
    assert(__fwk_module_get_ctx(MODULE0_ID)->element_count == 2);

    /* Check element 0 is correctly initialized */
    result = __fwk_module_get_state(ELEM0_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_STARTED);
    assert(__fwk_module_get_element_ctx(ELEM0_ID) ==
           &(__fwk_module_get_ctx(MODULE0_ID)->element_ctx_table[0]));
    assert(__fwk_module_get_element_ctx(ELEM0_ID)->desc ==
        &fake_element_desc_table0[0]);

    /* Check element 1 is correctly initialized */
    result = __fwk_module_get_state(ELEM1_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_STARTED);
    assert(__fwk_module_get_element_ctx(ELEM1_ID) ==
           &(__fwk_module_get_ctx(MODULE0_ID)->element_ctx_table[1]));
    assert(__fwk_module_get_element_ctx(ELEM1_ID)->desc ==
        &fake_element_desc_table0[1]);

    /* Check module 1 is correctly initialized */
    result = __fwk_module_get_state(MODULE1_ID, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_STARTED);
    assert(__fwk_module_get_ctx(MODULE1_ID)->desc == &fake_module_desc1);
    assert(__fwk_module_get_ctx(MODULE1_ID)->config == &fake_module_config1);
    assert(__fwk_module_get_ctx(MODULE1_ID)->element_count == 0);
}

static void test___fwk_module_init_succeed(void)
{
    int result;

    /* Remove non-mandatory functions from the description of module 1 */
    fake_module_desc1.post_init = NULL;
    fake_module_desc1.bind = NULL;
    fake_module_desc1.start = NULL;

    /* Module 1 has no element */
    fake_module_config1.get_element_table = NULL;

    bind_count_call = 0;
    start_count_call = 0;
    __fwk_module_reset();
    result = __fwk_module_init();
    assert(result == FWK_SUCCESS);
    /* bind is called twice for module 0 and twice for element 0 and 1 */
    assert(bind_count_call == 6);

    /* start is called once for module 0 and once for element 0 and 1 */
    assert(start_count_call == 3);
    check_correct_initialization();

    /* Init should failed if the initialisation has already been done */
    result = __fwk_module_init();
    assert(result == FWK_E_STATE);

    check_correct_initialization();
}

static void test___fwk_module_get_state(void)
{
    int result;
    fwk_id_t id;
    enum fwk_module_state state;

    /* Invalid ID */
    id = FWK_ID_ELEMENT(0xEF, 0xDBE);
    result = __fwk_module_get_state(id, &state);
    assert(result == FWK_E_PARAM);

    /* Bad parameters */
    result = __fwk_module_get_state(id, NULL);
    assert(result == FWK_E_PARAM);

    /* Get module 0 state */
    id = MODULE0_ID;
    result = __fwk_module_get_state(id, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_STARTED);

    /* Get module 1 state */
    id = MODULE1_ID;
    result = __fwk_module_get_state(id, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_STARTED);

    /* Get element 0 state */
    id = ELEM0_ID;
    result = __fwk_module_get_state(id, &state);
    assert(result == FWK_SUCCESS);
    assert(state == FWK_MODULE_STATE_STARTED);
}

static void test_fwk_module_is_valid_module_id(void)
{
    fwk_id_t id;
    bool result;

    /* Valid module ID */
    id = MODULE0_ID;
    result = fwk_module_is_valid_module_id(id);
    assert(result);

    /* Valid module ID */
    id = MODULE1_ID;
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

static void test_fwk_module_is_valid_element_id(void)
{
    fwk_id_t id;
    bool result;

    /* Valid element ID */
    id = ELEM0_ID;
    result = fwk_module_is_valid_element_id(id);
    assert(result);

    /* Valid element ID */
    id = ELEM1_ID;
    result = fwk_module_is_valid_element_id(id);
    assert(result);

    /* Module IDX non valid */
    id = FWK_ID_MODULE(0x03);
    result = fwk_module_is_valid_element_id(id);
    assert(!result);

    /* Element IDX non valid */
    id = FWK_ID_ELEMENT(MODULE1_IDX, 0x01);
    result = fwk_module_is_valid_element_id(id);
    assert(!result);

    /* Element IDX non valid */
    id = FWK_ID_ELEMENT(MODULE0_IDX, 0x02);
    result = fwk_module_is_valid_element_id(id);
    assert(!result);

    /* Invalid type */
    id = MODULE0_ID;
    result = fwk_module_is_valid_element_id(id);
    assert(!result);
}

static void test_fwk_module_is_valid_sub_element_id(void)
{
    bool result;

    /* Valid sub-element ID */
    result = fwk_module_is_valid_sub_element_id(SUB_ELEM0_ID);
    assert(result);

    /* Invalid type */
    result = fwk_module_is_valid_sub_element_id(NOTIFICATION0_ID);
    assert(!result);

    /* Invalid module IDX */
    result = fwk_module_is_valid_sub_element_id(FWK_ID_SUB_ELEMENT(5, ELEM0_IDX,
        SUB_ELEM0_IDX));
    assert(!result);

    /* Invalid element IDX */
    result = fwk_module_is_valid_sub_element_id(FWK_ID_SUB_ELEMENT(MODULE0_IDX,
        5, SUB_ELEM0_IDX));
    assert(!result);

    /* Invalid sub-element ID */
    result = fwk_module_is_valid_sub_element_id(FWK_ID_SUB_ELEMENT(MODULE0_IDX,
        ELEM0_IDX, 5));
    assert(!result);
}

static void test_fwk_module_is_valid_entity_id(void)
{
    bool result;

    /* Valid module ID */
    result = fwk_module_is_valid_entity_id(MODULE0_ID);
    assert(result);

    /* Invalid module ID */
    result = fwk_module_is_valid_entity_id(FWK_ID_MODULE(5));
    assert(!result);

    /* Valid element ID */
    result = fwk_module_is_valid_entity_id(ELEM0_ID);
    assert(result);

    /* Invalid element ID */
    result = fwk_module_is_valid_entity_id(FWK_ID_ELEMENT(5, 5));
    assert(!result);

    /* Valid sub-element ID */
    result = fwk_module_is_valid_entity_id(SUB_ELEM0_ID);
    assert(result);

    /* Invalid sub-element ID */
    result = fwk_module_is_valid_sub_element_id(FWK_ID_SUB_ELEMENT(5, 5, 5));
    assert(!result);

    /* Invalid entity as a notification is not an entity */
    result = fwk_module_is_valid_entity_id(NOTIFICATION0_ID);
    assert(!result);
}

static void test_fwk_module_is_valid_api_id(void)
{
    fwk_id_t id;
    bool result;

    /* Valid API ID */
    id = API0_ID;
    result = fwk_module_is_valid_api_id(id);
    assert(result);

    /* Valid API ID */
    id = API1_ID;
    result = fwk_module_is_valid_api_id(id);
    assert(result);

    /* Module IDX non valid */
    id = FWK_ID_MODULE(0x03);
    result = fwk_module_is_valid_api_id(id);
    assert(!result);

    /* API IDX non valid */
    id = FWK_ID_API(MODULE1_IDX, 0x01);
    result = fwk_module_is_valid_api_id(id);
    assert(!result);

    /* API IDX non valid */
    id = FWK_ID_ELEMENT(MODULE0_IDX, 0x02);
    result = fwk_module_is_valid_api_id(id);
    assert(!result);

    /* Invalid type */
    id = MODULE0_ID;
    result = fwk_module_is_valid_api_id(id);
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
    id = FWK_ID_EVENT(MODULE1_IDX, 0x03);
    result = fwk_module_is_valid_event_id(id);
    assert(!result);

    /* Module IDX non valid */
    id = FWK_ID_EVENT(MODULE0_IDX, 0x00);
    result = fwk_module_is_valid_event_id(id);
    assert(!result);

    /* Invalid type */
    id = MODULE0_ID;
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
    id = FWK_ID_NOTIFICATION(MODULE0_IDX, 0x03);
    result = fwk_module_is_valid_notification_id(id);
    assert(!result);

    /* Module IDX non valid */
    id = FWK_ID_NOTIFICATION(MODULE1_IDX, 0x00);
    result = fwk_module_is_valid_notification_id(id);
    assert(!result);

    /* Invalid type */
    id = MODULE0_ID;
    result = fwk_module_is_valid_notification_id(id);
    assert(!result);
}

static void test_fwk_module_get_element_count(void)
{
    int element_count;

    /* Valid module ID with 2 elements */
    element_count = fwk_module_get_element_count(MODULE0_ID);
    assert(element_count == 2);

    /* Invalid module ID */
    element_count = fwk_module_get_element_count(FWK_ID_MODULE(5));
    assert(element_count == FWK_E_PARAM);

    /* The function should fails as it expects a module ID */
    element_count = fwk_module_get_element_count(ELEM0_ID);
    assert(element_count == FWK_E_PARAM);
}

static void test_fwk_module_get_sub_element_count(void)
{
    int sub_element_count;

    /* Invalid element ID */
    sub_element_count =
        fwk_module_get_sub_element_count(FWK_ID_ELEMENT(MODULE0_IDX, 0x05));
    assert(sub_element_count == FWK_E_PARAM);

    /* Valid module ID, but not an element */
    sub_element_count = fwk_module_get_sub_element_count(MODULE0_ID);
    assert(sub_element_count == FWK_E_PARAM);

    /* Valid element ID with 0 sub-elements */
    sub_element_count = fwk_module_get_sub_element_count(ELEM1_ID);
    assert(sub_element_count == 0);

    /* Valid element ID with 1 sub-element */
    sub_element_count = fwk_module_get_sub_element_count(ELEM0_ID);
    assert(sub_element_count == 1);
}

static void test_fwk_module_get_name(void)
{
    fwk_id_t id;
    const char *result;

    /* Valid module ID */
    id = MODULE0_ID;
    result = fwk_module_get_name(id);
    assert(result != NULL);
    assert(strcmp(result, "FAKE MODULE 0") == 0);

    /* Valid element ID */
    id = ELEM0_ID;
    result = fwk_module_get_name(id);
    assert(result != NULL);
    assert(strcmp(result, "FAKE ELEM 0") == 0);

    /* Invalid ID */
    id = FWK_ID_ELEMENT(0xEF, 0xDBE);
    result = fwk_module_get_name(id);
    assert(result == NULL);
}

static void test_fwk_module_get_data(void)
{
    fwk_id_t id;
    const void *result;

    /* Valid module ID */
    id = MODULE0_ID;
    result = fwk_module_get_data(id);
    assert(result != NULL);
    assert(result == (void *)(&config_module0));

    /* Valid element ID */
    id = ELEM0_ID;
    result = fwk_module_get_data(id);
    assert(result != NULL);
    assert(result == (void *)(&config_elem0));

    /* Valid sub-element ID */
    id = SUB_ELEM0_ID;
    result = fwk_module_get_data(id);
    assert(result != NULL);
    assert(result == (void *)(&config_elem0));

    /* Invalid element ID */
     id = FWK_ID_ELEMENT(0xEF, 0xDBE);
     result = fwk_module_get_data(id);
     assert(result == NULL);

    /* Invalid sub-element ID */
    id = FWK_ID_SUB_ELEMENT(0xDE, 0xAA7, 9);
    result = fwk_module_get_data(id);
    assert(result == NULL);
}

static void test_fwk_module_bind_stage_failure(void)
{
    int result;
    struct fake_api api;

    /*
     * The framework is forced into the initialization stage
     */
    __fwk_module_reset();
    init_return_val = FWK_E_PARAM;
    result = __fwk_module_init();
    assert(result == FWK_E_PARAM);
    init_return_val = FWK_SUCCESS;

    /*
     * The binding request should fail because the framework is in
     * MODULE_STAGE_INITIALIZE and the module is in
     * FWK_MODULE_STATE_UNINITIALIZED state.
     */
    result = fwk_module_bind(MODULE0_ID, API1_ID, &api);
    assert(result == FWK_E_STATE);

    /*
     * The framework is forced into the start stage
     */
    start_return_val = FWK_E_PARAM;
    __fwk_module_reset();
    result = __fwk_module_init();
    assert(result == FWK_E_PARAM);
    start_return_val = FWK_SUCCESS;

    /*
     * The binding request should fail because it cannot be called when the
     * framework is in MODULE_STAGE_START stage.
     */
    process_bind_request_return_val = FWK_SUCCESS;
    result = fwk_module_bind(MODULE0_ID, API1_ID, &api);
    assert(result == FWK_E_STATE);
}

static void test_fwk_module_bind(void)
{
    int result;
    struct fake_api *api;
    void *null_api = NULL;

    /* The framework component is forced into the bound stage */
    __fwk_module_reset();
    bind_return_val = FWK_E_PARAM;
    result = __fwk_module_init();
    assert(result == FWK_E_PARAM);
    bind_return_val = FWK_SUCCESS;

    /* The binding request should fail because the target ID is not valid */
    result = fwk_module_bind(FWK_ID_MODULE(2), API0_ID, &api);
    assert(result == FWK_E_PARAM);

    /* The binding request should fail because the API ID is not valid */
    result = fwk_module_bind(MODULE1_ID, FWK_ID_API(1, 0), &api);
    assert(result == FWK_E_PARAM);

    /*
     * The binding request should fail because API0_ID does not belong to
     * MODULE1_ID.
     */
    result = fwk_module_bind(MODULE1_ID, API0_ID, &api);
    assert(result == FWK_E_PARAM);

    /*
     * The binding request should fail because the API address pointer is NULL
     */
    result = fwk_module_bind(MODULE0_ID, API1_ID, NULL);
    assert(result == FWK_E_PARAM);

    /*
     * The binding request should fail because the process_bind_request function
     * associated with the module fails.
     */
    process_bind_request_return_val = FWK_E_PARAM;
    result = fwk_module_bind(MODULE0_ID, API1_ID, &api);
    assert(result == FWK_E_PARAM);
    process_bind_request_return_val = FWK_SUCCESS;

    /*
     * The binding request should fail because the address of the API is
     * initially NULL and is not modified by the module process_bind_request()
     * function.
     */
    process_bind_request_return_api = false;
    result = fwk_module_bind(MODULE0_ID, API1_ID, &null_api);
    assert(result == FWK_E_HANDLER);
    assert(null_api == NULL);
    process_bind_request_return_api = true;

    /* The binding request should return successfully */
    result = fwk_module_bind(MODULE0_ID, API1_ID, &api);
    assert(result == FWK_SUCCESS);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test___fwk_module_init_module_desc_bad_params),
    FWK_TEST_CASE(test___fwk_module_init_failure),
    FWK_TEST_CASE(test___fwk_module_init_bind_failure),
    FWK_TEST_CASE(test___fwk_module_init_start_failure),
    FWK_TEST_CASE(test_fwk_thread_failure),
    FWK_TEST_CASE(test___fwk_module_init_succeed),
    FWK_TEST_CASE(test___fwk_module_get_state),
    FWK_TEST_CASE(test_fwk_module_is_valid_module_id),
    FWK_TEST_CASE(test_fwk_module_is_valid_element_id),
    FWK_TEST_CASE(test_fwk_module_is_valid_sub_element_id),
    FWK_TEST_CASE(test_fwk_module_is_valid_entity_id),
    FWK_TEST_CASE(test_fwk_module_is_valid_api_id),
    FWK_TEST_CASE(test_fwk_module_is_valid_event_id),
    FWK_TEST_CASE(test_fwk_module_is_valid_notification_id),
    FWK_TEST_CASE(test_fwk_module_get_element_count),
    FWK_TEST_CASE(test_fwk_module_get_sub_element_count),
    FWK_TEST_CASE(test_fwk_module_get_name),
    FWK_TEST_CASE(test_fwk_module_get_data),
    FWK_TEST_CASE(test_fwk_module_bind_stage_failure),
    FWK_TEST_CASE(test_fwk_module_bind)
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_module",
    .test_case_setup = test_case_setup,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
