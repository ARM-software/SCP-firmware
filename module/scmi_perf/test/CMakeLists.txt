#
# Arm SCP/MCP Software
# Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(TEST_MODULE scmi_perf)
include(${SCP_ROOT}/module/scmi_perf/test/module/mod_scmi_perf.cmake)

set(TEST_MODULE scmi_perf)
include(
    ${SCP_ROOT}/module/scmi_perf/test/plugins_handler/mod_scmi_perf_ph.cmake)

set(TEST_MODULE scmi_perf)
include(${SCP_ROOT}/module/scmi_perf/test/fastchannels/mod_scmi_perf_fch.cmake)

set(TEST_MODULE scmi_perf)
include(
    ${SCP_ROOT}/module/scmi_perf/test/protocol/mod_scmi_perf_protocol_ops.cmake)

set(BUILD_HAS_MOD_TRANSPORT_FC TRUE)
set(TEST_SUFFIX _transport_fc)

set(TEST_MODULE scmi_perf)
include(
    ${SCP_ROOT}/module/scmi_perf/test/plugins_handler/mod_scmi_perf_ph.cmake)

set(TEST_MODULE scmi_perf)
include(${SCP_ROOT}/module/scmi_perf/test/fastchannels/mod_scmi_perf_fch.cmake)

set(TEST_MODULE scmi_perf)
include(
    ${SCP_ROOT}/module/scmi_perf/test/protocol/mod_scmi_perf_protocol_ops.cmake)
