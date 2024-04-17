#
# Arm SCP/MCP Software
# Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Configure the build system.
#

set(SCP_FIRMWARE "scmi-fw")

set(SCP_FIRMWARE_TARGET "scmi-fw")

set(SCP_ARCHITECTURE "optee")

set(SCP_TOOLCHAIN_INIT "GNU")

set(CMAKE_BUILD_TYPE "Release")

set(SCP_ENABLE_NOTIFICATIONS_INIT TRUE)

set(SCP_ENABLE_SCMI_NOTIFICATIONS_INIT TRUE)

set(SCP_ENABLE_SCMI_SENSOR_EVENTS_INIT TRUE)

set(SCP_ENABLE_FAST_CHANNELS_INIT FALSE)

set(SCP_ENABLE_IPO_INIT FALSE)

# The order of the modules in the following list is the order in which the
# modules are initialized, bound, started during the pre-runtime phase.
# any change in the order will cause firmware initialization errors.

list(APPEND SCP_MODULES "optee-mbx")
list(APPEND SCP_MODULES "msg-smt")
list(APPEND SCP_MODULES "scmi")
list(APPEND SCP_MODULES "mock-clock")
list(APPEND SCP_MODULES "system-pll")
list(APPEND SCP_MODULES "clock")
list(APPEND SCP_MODULES "scmi-clock")
list(APPEND SCP_MODULES "mock-ppu")
list(APPEND SCP_MODULES "power-domain")
list(APPEND SCP_MODULES "scmi-power-domain")
list(APPEND SCP_MODULES "mock-psu")
list(APPEND SCP_MODULES "psu")
list(APPEND SCP_MODULES "dvfs")
list(APPEND SCP_MODULES "scmi-perf")
list(APPEND SCP_MODULES "reg-sensor")
list(APPEND SCP_MODULES "sensor")
list(APPEND SCP_MODULES "scmi-sensor")
list(APPEND SCP_MODULES "optee-console")
