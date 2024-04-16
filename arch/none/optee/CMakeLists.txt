#
# Arm SCP/MCP Software
# Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(arch-optee)

# OP-TEE CMake switches
set (CFG_SCPFW_MOD_DVFS n)
set (CFG_SCPFW_MOD_CLOCK n)
set (CFG_SCPFW_MOD_MOCK_CLOCK n)
set (CFG_SCPFW_MOD_MOCK_PPU n)
set (CFG_SCPFW_MOD_MOCK_PSU n)
set (CFG_SCPFW_MOD_MSG_SMT n)
set (CFG_SCPFW_MOD_OPTEE_CONSOLE n)
set (CFG_SCPFW_MOD_OPTEE_CLOCK n)
set (CFG_SCPFW_MOD_OPTEE_MBX n)
set (CFG_SCPFW_MOD_PSU_OPTEE_REGULATOR n)
set (CFG_SCPFW_MOD_OPTEE_RESET n)
set (CFG_SCPFW_MOD_OPTEE_SMT n)
set (CFG_SCPFW_MOD_OPTEE_VOLTD_REGULATOR n)
set (CFG_SCPFW_MOD_POWER_DOMAIN n)
set (CFG_SCPFW_MOD_PSU n)
set (CFG_SCPFW_MOD_REG_SENSOR n)
set (CFG_SCPFW_MOD_RESET_DOMAIN n)
set (CFG_SCPFW_MOD_SCMI n)
set (CFG_SCPFW_MOD_SCMI_APCORE n)
set (CFG_SCPFW_MOD_SCMI_CLOCK n)
set (CFG_SCPFW_MOD_SCMI_PERF n)
set (CFG_SCPFW_MOD_SCMI_POWER_DOMAIN n)
set (CFG_SCPFW_MOD_SCMI_RESET_DOMAIN n)
set (CFG_SCPFW_MOD_SCMI_SENSOR n)
set (CFG_SCPFW_MOD_SCMI_VOLTAGE_DOMAIN n)
set (CFG_SCPFW_MOD_SENSOR n)
set (CFG_SCPFW_MOD_SYSTEM_PLL n)
set (CFG_SCPFW_MOD_VOLTAGE_DOMAIN n)

if(NOT CFG_NUM_THREADS)
        message(FATAL_ERROR
            "Insufficient firmware metadata provided.\n"
            "For optee arch, you must set CFG_NUM_THREADS")
endif()

if(NOT SCP_OPTEE_DIR)
        message(FATAL_ERROR
            "Insufficient firmware metadata provided.\n"
            "For optee arch, you must set SCP_OPTEE_DIR")
endif()

target_compile_definitions(arch-optee PUBLIC -DCFG_NUM_THREADS=${CFG_NUM_THREADS})

target_include_directories(arch-optee
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(arch-optee
                      PUBLIC module-optee-mbx)

target_sources(
    arch-optee PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_interrupt.c"
                       "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_main.c")
