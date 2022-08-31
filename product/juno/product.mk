#
# Arm SCP/MCP Software
# Copyright (c) 2019-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_PRODUCT_NAME := juno

ifeq ($(TEST_ON_TARGET),1)
    BS_FIRMWARE_LIST := scp_romfw \
                        scp_romfw_bypass \
                        scp_ut
else
    BS_FIRMWARE_LIST := scp_romfw_bypass \
                        scp_ramfw

    ifdef PLATFORM_VARIANT
        ifeq ($(PLATFORM_VARIANT), FVP)
            BS_FIRMWARE_LIST := scp_romfw \
                                scp_ramfw
        endif
    endif
endif
