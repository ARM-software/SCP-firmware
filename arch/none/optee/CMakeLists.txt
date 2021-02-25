#
# Arm SCP/MCP Software
# Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(arch-optee)

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
