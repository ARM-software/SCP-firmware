#
# Arm SCP/MCP Software
# Copyright (c) 2024, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# SCMI server in OPTEE is built from SCP-firmware source tree.
# The firmware is made of a framework, a product and modules.
# Only modules used by firmware must be built, as stated by
# CFG_SCPFW_MOD_* swtches. SCP-firmware needs a C source and
# a header file to be generated to describe embedded modules.
# This is done through cmake configuration of the package.
# The toolchain build directive must also match the list of
# embedded modules.

# Optee Architecture files
incdirs_ext-y += $(scpfw-path)/arch/none/optee/include
srcs-y += $(scpfw-path)/arch/none/optee/src/arch_interrupt.c
srcs-y += $(scpfw-path)/arch/none/optee/src/arch_main.c

# Architecture arch/none/optee requires optee mbx header file
incdirs_ext-y += $(scpfw-path)/product/optee/common/module/mbx/include

# Framework files
incdirs_ext-y += $(scpfw-path)/framework/include
srcs-y += $(scpfw-path)/framework/src/fwk_arch.c
srcs-y += $(scpfw-path)/framework/src/fwk_dlist.c
srcs-y += $(scpfw-path)/framework/src/fwk_id.c
srcs-y += $(scpfw-path)/framework/src/fwk_interrupt.c
srcs-y += $(scpfw-path)/framework/src/fwk_io.c
srcs-y += $(scpfw-path)/framework/src/fwk_log.c
srcs-y += $(scpfw-path)/framework/src/fwk_mm.c
srcs-y += $(scpfw-path)/framework/src/fwk_module.c
srcs-y += $(scpfw-path)/framework/src/fwk_ring.c
srcs-y += $(scpfw-path)/framework/src/fwk_slist.c
srcs-y += $(scpfw-path)/framework/src/fwk_status.c
srcs-y += $(scpfw-path)/framework/src/fwk_string.c
srcs-y += $(scpfw-path)/framework/src/fwk_delayed_resp.c
srcs-y += $(scpfw-path)/framework/src/fwk_time.c
srcs-y += $(scpfw-path)/framework/src/fwk_core.c
srcs-y += $(scpfw-path)/framework/src/assert.c
srcs-y += $(scpfw-path)/framework/src/stdlib.c
srcs-$(CFG_SCPFW_NOTIFICATION) += $(scpfw-path)/framework/src/fwk_notification.c

# Helper macros for listing SCP-firmware modules source files (in srcs-y)
# and header include paths (in incdirs_ext-y). Each module provides a C source
# file named mod_<module-name>.c and possibly an include directory. Build
# directive BUILD_HAS_MOD_<NAME> must be set for each embedded module.
#
# Standard modules source tree: <scp-path>/module/<name>/src/mod_<name>.c
# Optee modules source tree:    <scp-path>/product/common/module/<short-name>/src/mod_<name>.c
# Product modules source tree:  <scp-path>/product/<product-name>/module/<name>/src/mod_<name>.c
#
# scpfw-embed-generic-module is to be used for standard modules.
# scpfw-embed-optee-module is to be used for optee modules.
# scpfw-embed-product-module is to be used for product modules.
# For modules that implement other C source files aside mandatory mod_<name>.c we must
# add to srcs-y the required source file paths.
#
# scpfw-embed-mod takes 4 arguments:
# $1 module name, lowercase
# $2 module directory name
# $3 module parent directory relative path in scpfw tree
# $4 module name, uppercase, relates to CFG_SCPFW_MOD_$4
define scpfw-embed-mod
ifneq (,$$(wildcard $(scpfw-path)/$3/$2/include/*))
incdirs_ext-y += $(scpfw-path)/$3/$2/include
endif
srcs-$(CFG_SCPFW_MOD_$4) += $(scpfw-path)/$3/$2/src/mod_$1.c

# SCMI_Perf in SCP-firmware has components that can be added conditionally at
# build time.
ifeq ($(1), scmi_perf)

ifeq ($(CFG_SCPFW_SCMI_PERF_PROTOCOL_OPS),y)
srcs-$(CFG_SCPFW_MOD_SCMI_PERF) += $(scpfw-path)/$3/$2/src/scmi_perf_protocol_ops.c
endif

ifeq ($(CFG_SCPFW_SCMI_PERF_FAST_CHANNELS),y)
srcs-$(CFG_SCPFW_MOD_SCMI_PERF) += $(scpfw-path)/$3/$2/src/scmi_perf_fastchannels.c
endif

endif

cflags-lib-$(CFG_SCPFW_MOD_$4) += -DBUILD_HAS_MOD_$4
endef

define scpfw-embed-generic-module
$(eval $(call scpfw-embed-mod,$1,$1,module,$(shell echo $1 | tr a-z A-Z)))
endef

define scpfw-embed-optee-module
$(eval $(call scpfw-embed-mod,optee_$1,$1,product/optee/common/module,OPTEE_$(shell echo $1 | tr a-z A-Z)))
endef

define scpfw-embed-product-module
$(eval $(call scpfw-embed-mod,$1,$1,product/$(scpfw-product)/module,$(shell echo $1 | tr a-z A-Z)))
endef

# Modules supported by OPTEE
$(eval $(call scpfw-embed-generic-module,clock))
$(eval $(call scpfw-embed-generic-module,dvfs))
$(eval $(call scpfw-embed-generic-module,mock_clock))
$(eval $(call scpfw-embed-generic-module,mock_ppu))
$(eval $(call scpfw-embed-generic-module,mock_psu))
$(eval $(call scpfw-embed-generic-module,msg_smt))
$(eval $(call scpfw-embed-generic-module,power_domain))
$(eval $(call scpfw-embed-generic-module,psu))
$(eval $(call scpfw-embed-generic-module,reg_sensor))
$(eval $(call scpfw-embed-generic-module,reset_domain))
$(eval $(call scpfw-embed-generic-module,sensor))
$(eval $(call scpfw-embed-generic-module,scmi))
$(eval $(call scpfw-embed-generic-module,scmi_apcore))
$(eval $(call scpfw-embed-generic-module,scmi_clock))
$(eval $(call scpfw-embed-generic-module,scmi_perf))
$(eval $(call scpfw-embed-generic-module,scmi_power_domain))
$(eval $(call scpfw-embed-generic-module,scmi_reset_domain))
$(eval $(call scpfw-embed-generic-module,scmi_sensor))
$(eval $(call scpfw-embed-generic-module,scmi_voltage_domain))
$(eval $(call scpfw-embed-generic-module,system_pll))
$(eval $(call scpfw-embed-generic-module,voltage_domain))
$(eval $(call scpfw-embed-optee-module,clock))
$(eval $(call scpfw-embed-optee-module,console))
$(eval $(call scpfw-embed-optee-module,mbx))
$(eval $(call scpfw-embed-optee-module,reset))
$(eval $(call scpfw-embed-optee-module,smt))

# Some modules have extra and non generic C files
srcs-$(CFG_SCPFW_MOD_CLOCK) += $(scpfw-path)/module/clock/src/clock_tree_management.c
srcs-$(CFG_SCPFW_MOD_POWER_DOMAIN) += $(scpfw-path)/module/power_domain/src/power_domain_notifications.c
srcs-$(CFG_SCPFW_MOD_POWER_DOMAIN) += $(scpfw-path)/module/power_domain/src/power_domain_state_checks.c
srcs-$(CFG_SCPFW_MOD_POWER_DOMAIN) += $(scpfw-path)/module/power_domain/src/power_domain_utils.c
srcs-$(CFG_SCPFW_MOD_SCMI) += $(scpfw-path)/module/scmi/src/mod_scmi_base.c
srcs-$(CFG_SCPFW_MOD_SCMI_SENSOR) += $(scpfw-path)/module/scmi_sensor/src/mod_scmi_ext_attrib.c
srcs-$(CFG_SCPFW_MOD_SENSOR) += $(scpfw-path)/module/sensor/src/sensor_extended.c

# Some modules require header files from module that are not embedded
ifneq (,$(filter y, $(CFG_SCPFW_MOD_DVFS) $(CFG_SCPFW_MOD_MOCK_PSU) $(CFG_SCPFW_MOD_SCMI_PERF)))
incdirs_ext-y += $(scpfw-path)/module/timer/include
endif
incdirs_ext-$(CFG_SCPFW_MOD_OPTEE_MBX) += $(scpfw-path)/module/msg_smt/include
incdirs_ext-$(CFG_SCPFW_MOD_SCMI) += $(scpfw-path)/module/power_domain/include

# Product specific files
include $(scpfw-path)/product/$(scpfw-product)/sub-$(CFG_SCMI_SCPFW_PRODUCT).mk
