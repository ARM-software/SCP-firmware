/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Unity test framework needs the mocked version of the functions which are
 * called by the function under test, e.g. in this case
 * mutex_init and mutex_unlock.
 * Normally mocked version of the function can be generated using
 * CMock tool(see unit_test/user_guide.md)
 * by providing the header that includes declarations. However, in a few cases
 * it would need other header files as well complicating the Mocks generation.
 * This file demonstrates, how to achieve requires mocks generation without
 * much dependency.
 */

#include <optee_scmi.h>

uintptr_t smt_phys_to_virt(uintptr_t pa, size_t sz, bool shmem_is_secure);
