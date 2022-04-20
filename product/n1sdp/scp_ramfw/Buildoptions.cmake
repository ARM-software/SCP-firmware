#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_dependent_option(
    SCP_N1SDP_SENSOR_LIB_PATH "Path of the n1sdp sensor library"
    "${SCP_N1SDP_SENSOR_LIB_PATH}" "DEFINED SCP_N1SDP_SENSOR_LIB_PATH" OFF)
