/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_TEST_H
#define FWK_TEST_H

/*!
 * \addtogroup GroupLibFramework Framework
 * \{
 */

/*!
 * \defgroup GroupTest Test
 *
 * \{
 */

/*!
 * \brief Define a test case description.
 *
 * \param FUNC Test case function name.
 *
 * \return A test case description.
 */
#define FWK_TEST_CASE(FUNC) { .name = #FUNC, .test_execute = FUNC }

/*!
 * \brief Test case description.
 */
struct fwk_test_case_desc {
    /*! Test case name */
    const char *name;

    /*!
     * \brief Pointer to the test case execution function.
     *
     * \return None.
     *
     * \note A test case is identified as having successfully completed if
     *      execution returns from this function. Test case execution functions
     *      should use the assert() macro from the C standard library to check
     *      test conditions.
     */
    void (*test_execute)(void);
};

/*!
 * \brief Test suite description.
 */
struct fwk_test_suite_desc {
    /*! Test suite name */
    const char *name;

    /*!
     * \brief Pointer to a test suite setup function.
     *
     * \details This function should be used to initialize and configure a test
     *      fixture or to execute expensive routines that could otherwise be
     *      done within a test case setup function.
     *
     * \retval ::FWK_SUCCESS The test suite environment was successfully set up.
     * \return Any of the other error codes defined by the framework.
     *
     * \note May be NULL, in which case the test suite is considered to have no
     *      setup function. In the event that test suite setup fails, the test
     *      suite is not executed.
     */
    int (*test_suite_setup)(void);

    /*!
     * \brief Pointer to a test suite teardown function.
     *
     * \return None.
     *
     * \note May be NULL, in which case the test suite is considered to have no
     *      teardown function.
     */
    void (*test_suite_teardown)(void);

    /*!
     * \brief Pointer to a test case setup function.
     *
     * \details This function should be used to ensure test cases are running in
     *      a known, sane environment prior to execution.
     *
     * \return None.
     *
     * \note May be NULL, in which case the test case is considered to have no
     *      setup function.
     */
    void (*test_case_setup)(void);

    /*!
     * \brief Pointer to a test case teardown function.
     *
     * \return None.
     *
     * \note May be NULL, in which case the test case is considered to have no
     *      teardown function.
     */
    void (*test_case_teardown)(void);

    /*! Number of test cases */
    unsigned int test_case_count;

    /*! Pointer to array of test cases */
    const struct fwk_test_case_desc *test_case_table;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* FWK_TEST_H */
