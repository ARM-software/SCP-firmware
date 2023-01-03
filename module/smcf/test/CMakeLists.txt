#
# Arm SCP/MCP Software
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(TEST_MODULE smcf)
include(${SCP_ROOT}/module/smcf/test/mgi/mod_smcf_mgi.cmake)

set(TEST_MODULE smcf)
include(${SCP_ROOT}/module/smcf/test/module/mod_smcf.cmake)

set(TEST_MODULE smcf)
include(${SCP_ROOT}/module/smcf/test/smcf_data/mod_smcf_data.cmake)
