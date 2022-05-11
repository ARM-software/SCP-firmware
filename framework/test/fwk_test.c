/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_noreturn.h>
#include <fwk_status.h>
#include <fwk_test.h>

#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
/*
 * This variable is used by arm architecture to ensure spurious nested calls
 * won't enable interrupts. This is been accessed from inline function defined
 * in arch_helpers.h
 */
unsigned int critical_section_nest_level;

/* Test information provided by the test suite */
extern struct fwk_test_suite_desc test_suite;

static jmp_buf test_buf_context;

struct fwk_module *module_table[FWK_MODULE_IDX_COUNT];
struct fwk_module_config *module_config_table[FWK_MODULE_IDX_COUNT];

static int fake_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    return FWK_SUCCESS;
}

noreturn void __assert_fail(const char *assertion,
    const char *file, unsigned int line, const char *function)
{
    printf("Assertion failed: %s\n", assertion);
    printf("    Function: %s\n", function);
    printf("    File: %-66s\n", file);
    printf("    Line: %u\n", line);
    longjmp(test_buf_context, !FWK_SUCCESS);
}

static void print_separator(void)
{
    printf("----------------------------------------");
    printf("----------------------------------------\n");
}
static void print_prologue(void)
{
    printf("\nStarting tests for %s\n", test_suite.name);
    print_separator();
}

static void print_epilogue(unsigned int successful_tests)
{
    int pass_rate = (successful_tests * 100) / test_suite.test_case_count;

    print_separator();
    printf("%u / %u passed (%d%% pass rate)\n\n", successful_tests,
        test_suite.test_case_count, pass_rate);
}

static void print_result(const char *name, bool success)
{
    /* The name is truncated to 72 characters */
    printf("%-72s %s\n", name, (success ? "SUCCESS" : "FAILURE"));
}

static unsigned int run_tests(void)
{
    unsigned int i;
    bool success;
    unsigned int successful_tests = 0;
    const struct fwk_test_case_desc *test_case;

    if (test_suite.test_suite_setup != NULL) {
        if (test_suite.test_suite_setup() != FWK_SUCCESS)
            return 0;
    }

    for (i = 0; i < test_suite.test_case_count; i++) {
        test_case = &test_suite.test_case_table[i];

        if ((test_case->test_execute == NULL) || (test_case->name == NULL)) {
            print_result("Test case undefined!", false);

            continue;
        }

        if (test_suite.test_case_setup != NULL)
            test_suite.test_case_setup();

        /*
         * The setjmp function stores the execution context of the processor at
         * that point in time. When called, 0 is returned by default.
         * If an assertion fails in the test case following, execution returns
         * to an undefined point within setjmp() which then returns a non-zero
         * value. See __assert_fail() for exactly how assertion failure is
         * handled.
         */
        if (setjmp(test_buf_context) == FWK_SUCCESS) {
            test_case->test_execute();

            success = true;
            successful_tests++;
        } else
            success = false;

        if (test_suite.test_case_teardown != NULL)
            test_suite.test_case_teardown();

        print_result(test_case->name, success);
    }

    if (test_suite.test_suite_teardown != NULL)
        test_suite.test_suite_teardown();

    return successful_tests;
}

int main(void)
{
    unsigned int successful_tests;

    for (enum fwk_module_idx i = 0; i < FWK_MODULE_IDX_COUNT; i++) {

        struct fwk_module *module = malloc(sizeof(*module));
        struct fwk_module_config *config = malloc(sizeof(*config));

        *module = (struct fwk_module){
            .type = FWK_MODULE_TYPE_SERVICE,
            .init = fake_init,
        };

        *config = (struct fwk_module_config){ 0 };

        module_table[i] = module;
        module_config_table[i] = config;
    }

    fwk_module_init();

    if (test_suite.test_case_count != 0) {
        print_prologue();
        successful_tests = run_tests();
        print_epilogue(successful_tests);

        if (successful_tests != test_suite.test_case_count)
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
