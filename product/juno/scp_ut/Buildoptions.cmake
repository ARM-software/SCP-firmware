#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_dependent_option(
    SCP_ENABLE_SCMI_RESET "Enable the scmi reset?"
    "${SCP_ENABLE_SCMI_RESET_INIT}" "DEFINED SCP_ENABLE_SCMI_RESET_INIT"
    "${SCP_ENABLE_SCMI_RESET}")

cmake_dependent_option(
    SCP_ENABLE_FAST_CHANNELS "Enable the SCMI Fast channels?"
    "${SCP_ENABLE_FAST_CHANNELS_INIT}" "DEFINED SCP_ENABLE_FAST_CHANNELS_INIT"
    "${SCP_ENABLE_FAST_CHANNELS}")

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
