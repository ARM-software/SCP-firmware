#
# Renesas SCP/MCP Software
# Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_FIRMWARE_CPU := cortex-a57.cortex-a53
BS_FIRMWARE_HAS_MULTITHREADING := yes
BS_FIRMWARE_HAS_NOTIFICATION := yes
BS_FIRMWARE_HAS_SCMI_NOTIFICATIONS := no
BS_FIRMWARE_HAS_FAST_CHANNELS := no
BS_FIRMWARE_HAS_RESOURCE_PERMISSIONS := no
BS_FIRMWARE_HAS_SCMI_RESET := yes

BS_FIRMWARE_MODULES := \
    rcar_scif \
    rcar_system \
    scmi \
    smt \
    clock \
    rcar_clock \
    rcar_sd_clock \
    rcar_mstp_clock \
    rcar_ext_clock \
    rcar_system_power \
    dvfs \
    psu \
    rcar_arch_timer \
    timer \
    rcar_pmic \
    rcar_mock_pmic_bd9571 \
    rcar_mfismh \
    power_domain \
    rcar_pd_sysc \
    rcar_pd_core \
    rcar_pd_pmic_bd9571 \
    rcar_reg_sensor \
    sensor \
    reset_domain \
    rcar_reset \
    scmi_perf \
    scmi_power_domain \
    scmi_clock \
    scmi_sensor \
    scmi_system_power \
    scmi_apcore \
    scmi_reset_domain \
    sds

ifeq ($(BS_FIRMWARE_HAS_RESOURCE_PERMISSIONS),yes)
    BS_FIRMWARE_MODULES += resource_perms
endif

BS_FIRMWARE_SOURCES := \
    rcar_core.c \
    config_rcar_scif.c \
    config_power_domain.c \
    config_rcar_pd_sysc.c \
    config_rcar_pd_core.c \
    config_rcar_pd_pmic_bd9571.c \
    config_sensor.c \
    config_clock.c \
    config_rcar_clock.c \
    config_rcar_sd_clock.c \
    config_rcar_mstp_clock.c \
    config_rcar_ext_clock.c \
    config_dvfs.c \
    config_psu.c \
    config_rcar_pmic.c \
    config_rcar_mock_pmic_bd9571.c \
    config_rcar_mfismh.c \
    config_timer.c \
    config_smt.c \
    config_scmi.c \
    config_scmi_perf.c \
    config_scmi_system_power.c \
    config_scmi_clock.c \
    config_scmi_apcore.c \
    config_scmi_power_domain.c \
    config_scmi_reset_domain.c \
    config_reset_domain.c \
    config_rcar_reset.c \
    config_rcar_system_power.c \
    config_rcar_system.c \
    config_sds.c

ifeq ($(BS_FIRMWARE_HAS_RESOURCE_PERMISSIONS),yes)
    BS_FIRMWARE_SOURCES += config_resource_perms.c
endif

#
# Temporary source code until CMSIS-FreeRTOS is updated
#
BS_FIRMWARE_SOURCES += \
    portASM.S \
    port.c \
    list.c \
    queue.c \
    tasks.c \
    timers.c \
    heap_1.c \
    cmsis_os2_tiny4scp.c

vpath %.c $(PRODUCT_DIR)/src/CMSIS-FreeRTOS/Source/portable/GCC/ARM_CA53_64_Rcar
vpath %.S $(PRODUCT_DIR)/src/CMSIS-FreeRTOS/Source/portable/GCC/ARM_CA53_64_Rcar
vpath %.c $(PRODUCT_DIR)/src/CMSIS-FreeRTOS/Source
vpath %.c $(PRODUCT_DIR)/src/CMSIS-FreeRTOS/Source/portable/MemMang
vpath %.c $(PRODUCT_DIR)/src/CMSIS-FreeRTOS/CMSIS/RTOS2/FreeRTOS/Source

#
# Temporary directory until CMSIS-FreeRTOS is updated
#
FREERTOS_DIR := $(PRODUCT_DIR)/src/CMSIS-FreeRTOS/CMSIS/RTOS2

include $(BS_DIR)/firmware.mk
