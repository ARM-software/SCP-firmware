#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Select SPI-NOR flash memory
#
DEVICE_NOR = MACRONIX

#
# set build target and macro definition for SPI-NOR
#
ifeq ($(DEVICE_NOR),MACRONIX)
BS_FIRMWARE_SOURCES += device_nor_mx25.c
DEFINES += DEVICE_NOR_MX25
endif
