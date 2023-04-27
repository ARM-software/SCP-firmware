#
# Arm SCP/MCP Software
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_dependent_option(
    SCP_ENABLE_SCMI_PERF_FAST_CHANNELS "Enable the SCMI-perf Fast Channels ?"
    "${SCP_ENABLE_SCMI_PERF_FAST_CHANNELS_INIT}"
    "DEFINED SCP_ENABLE_SCMI_PERF_FAST_CHANNELS_INIT"
    "${SCP_ENABLE_SCMI_PERF_FAST_CHANNELS}")

cmake_dependent_option(
    SCP_ENABLE_PLUGIN_HANDLER "Enable the Performance Plugins-Handler ?"
    "${SCP_ENABLE_PLUGIN_HANDLER_INIT}" "DEFINED SCP_ENABLE_PLUGIN_HANDLER_INIT"
    "${SCP_ENABLE_PLUGIN_HANDLER}")
