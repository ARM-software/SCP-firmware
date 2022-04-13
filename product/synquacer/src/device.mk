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
#DEVICE_NOR = MICRON
#DEVICE_NOR = INFINEON
#DEVICE_NOR = WINBOND

#
# set build target and macro definition for SPI-NOR
#
ifeq ($(DEVICE_NOR),MACRONIX)
BS_FIRMWARE_SOURCES += device_nor_mx25.c
DEFINES += DEVICE_NOR_MX25
else ifeq ($(DEVICE_NOR),MICRON)
BS_FIRMWARE_SOURCES += device_nor_mt25.c
DEFINES += DEVICE_NOR_MT25
else ifeq ($(DEVICE_NOR),INFINEON)
BS_FIRMWARE_SOURCES += device_nor_s25.c
DEFINES += DEVICE_NOR_S25
else ifeq ($(DEVICE_NOR),WINBOND)
BS_FIRMWARE_SOURCES += device_nor_w25.c
DEFINES += DEVICE_NOR_W25
endif
