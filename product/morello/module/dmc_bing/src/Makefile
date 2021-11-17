#
# Arm SCP/MCP Software
# Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_LIB_NAME := mod_dmc_bing
ifeq ($(PLAT_FVP), 1)
BS_LIB_SOURCES += mod_dmc_bing.c
else
BS_LIB_SOURCES += \
    mod_dmc_bing.c \
    dimm_spd.c \
    morello_ddr_phy.c \
    ddr_phy_values_800.c \
    ddr_phy_values_1200.c \
    ddr_phy_values_1333.c \
    ddr_phy_values_1466.c
endif

include $(BS_DIR)/lib.mk
