#
# Arm SCP/MCP Software
# Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_dependent_option(
    SCP_ENABLE_SCMI_RESET "Enable the scmi reset?"
    "${SCP_ENABLE_SCMI_RESET_INIT}" "DEFINED SCP_ENABLE_SCMI_RESET_INIT"
    "${SCP_ENABLE_SCMI_RESET}")

cmake_dependent_option(
    SCP_ENABLE_SCMI_PERF_FAST_CHANNELS "Enable the SCMI-perf Fast channels?"
    "${SCP_ENABLE_SCMI_PERF_FAST_CHANNELS_INIT}"
    "DEFINED SCP_ENABLE_SCMI_PERF_FAST_CHANNELS_INIT"
    "${SCP_ENABLE_SCMI_PERF_FAST_CHANNELS}")

if(SCP_ENABLE_SCMI_PERF_FAST_CHANNELS)
    if(NOT DEFINED BUILD_HAS_MOD_TRANSPORT_FC)
        option(BUILD_HAS_MOD_TRANSPORT_FC
               "SCMI-PERF Fast Channel default implementation is transport based" ON)
    endif()
endif()

cmake_dependent_option(
    SCP_ENABLE_DEBUG_UNIT "Enable the debug support?"
    "${SCP_ENABLE_DEBUG_UNIT_INIT}" "DEFINED SCP_ENABLE_DEBUG_UNIT_INIT"
    "${SCP_ENABLE_DEBUG_UNIT}")

cmake_dependent_option(
    SCP_ENABLE_STATISTICS "Enable the performance statistics?"
    "${SCP_ENABLE_STATISTICS_INIT}" "DEFINED SCP_ENABLE_STATISTICS_INIT"
    "${SCP_ENABLE_STATISTICS}")

cmake_dependent_option(
    SCP_ENABLE_PMI "Enable the performance measurement and instrumentation?"
    "${SCP_ENABLE_PMI_INIT}" "DEFINED SCP_ENABLE_PMI_INIT"
    "${SCP_ENABLE_PMI}")
