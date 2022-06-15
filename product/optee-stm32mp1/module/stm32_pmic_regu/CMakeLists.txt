#
# Arm SCP/MCP Software
# Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                                  "${SCP_OPTEE_DIR}/core/arch/arm/plat-stm32mp1")

target_sources(
    ${SCP_MODULE_TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_stm32_pmic_regu.c")

target_link_libraries(${SCP_MODULE_TARGET}
                        PUBLIC module-scmi module-voltage-domain)
