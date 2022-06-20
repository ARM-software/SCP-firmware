#
# Arm SCP/MCP Software
# Copyright (c) 2019-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_PRODUCT_NAME := juno

ifeq ($(TEST_ON_TARGET),1)
    BS_FIRMWARE_LIST := scp_romfw \
                        scp_romfw_bypass \
                        scp_ut
else
    BS_FIRMWARE_LIST := scp_romfw \
                        scp_romfw_bypass \
                        scp_ramfw
endif
