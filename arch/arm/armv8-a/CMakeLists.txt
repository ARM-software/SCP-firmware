#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(arch-armv8a)

target_include_directories(
    arch-armv8a
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/lib")

# cmake-lint: disable=E1122

target_sources(
    arch-armv8a
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_cache_helpers.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_crt0.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_exceptions.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_gic.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_main.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_misc_helpers.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_mm.c")

# Force the linker to include any object files containing our implementations of
# functions that the standard library relies on.
#

if(SCP_HAVE_NEWLIB)
    target_compile_definitions(arch-armv8a PUBLIC -DUSE_NEWLIB)
    target_link_options(arch-armv8a PUBLIC "LINKER:--undefined=_sbrk")
endif()

#
# Preprocess the linker script.
#
# CMake provides the `CMAKE_C_CREATE_PREPROCESSED_SOURCE` variable, which
# describes the command line required to preprocess a C source file. This
# variable is in a format similar to this (verbatim):
#
# <CMAKE_C_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -E <SOURCE> >
# <PREPROCESSED_SOURCE>
#
# We do some processing on this variable to convert these bracket-surrounded
# names to variables we set. For example, `<DEFINES>` is replaced with
# `${CPP_DEFINES}`. We then need to do some magic to expand that string out to
# the value of the actual variable.
#

include(SCPTargetLinkerScript)

set(scp_lds "${CMAKE_CURRENT_SOURCE_DIR}/src/arch.ld.S")

scp_target_linker_script(arch-armv8a "${scp_lds}")
