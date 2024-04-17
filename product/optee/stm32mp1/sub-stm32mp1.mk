#
# Arm SCP/MCP Software
# Copyright (c) 2024, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

incdirs_ext-y += $(scpfw-path)/product/optee/stm32mp1/include

srcs-y += $(scpfw-path)/product/optee/stm32mp1/fw/config_mbx_smt.c
srcs-y += $(scpfw-path)/product/optee/stm32mp1/fw/config_scmi.c
srcs-y += $(scpfw-path)/product/optee/stm32mp1/fw/config_scmi_clocks.c
srcs-y += $(scpfw-path)/product/optee/stm32mp1/fw/config_scmi_reset_domains.c
srcs-y += $(scpfw-path)/product/optee/stm32mp1/fw/config_scmi_voltage_domains.c

$(eval $(call scpfw-embed-product-module,stm32_pmic_regu))
$(eval $(call scpfw-embed-product-module,stm32_pwr_regu))
