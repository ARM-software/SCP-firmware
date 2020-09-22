#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(arch-armv7m)

target_include_directories(arch-armv7m
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# cmake-lint: disable=E1122

target_sources(
    arch-armv7m
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_exceptions.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_handlers.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_main.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/arch_nvic.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src/arch_mm.c")

target_link_libraries(arch-armv7m PUBLIC cmsis::core-m)

if(SCP_ENABLE_MULTITHREADING)
    target_link_libraries(arch-armv7m PRIVATE cmsis::rtos2-rtx)
endif()

#
# Select the standard library, if we have the option.
#

include(CMakeDependentOption)

cmake_dependent_option(
    SCP_ENABLE_NEWLIB_NANO "Enable newlib-nano instead of newlib?" TRUE
    "CMAKE_C_COMPILER_ID STREQUAL GNU" FALSE)

if(SCP_ENABLE_NEWLIB_NANO)
    target_link_options(arch-armv7m PUBLIC "--specs=nano.specs")
endif()

#
# Force the linker to include any object files containing our implementations of
# functions that the standard library relies on.
#

target_link_options(arch-armv7m PUBLIC "LINKER:--undefined=arch_exceptions")

if(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
    target_link_options(arch-armv7m
                        PUBLIC "LINKER:--entry=arch_exception_reset")
endif()

if(SCP_HAVE_NEWLIB)
    target_link_options(arch-armv7m PUBLIC "LINKER:--undefined=posix_memalign"
                                           "LINKER:--undefined=_sbrk")
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

if(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
    set(scp_lds "${CMAKE_CURRENT_SOURCE_DIR}/src/arch.scatter.S")
else()
    set(scp_lds "${CMAKE_CURRENT_SOURCE_DIR}/src/arch.ld.S")
endif()

scp_target_linker_script(arch-armv7m "${scp_lds}")
