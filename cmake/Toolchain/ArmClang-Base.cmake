#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(CMAKE_ASM_COMPILER
    armclang
    CACHE FILEPATH "Path to the assembler.")
set(CMAKE_C_COMPILER
    armclang
    CACHE FILEPATH "Path to the C compiler.")
set(CMAKE_CXX_COMPILER
    armclang
    CACHE FILEPATH "Path to the C++ compiler.")

foreach(language IN ITEMS ASM C CXX)
    if(CMAKE_${language}_COMPILER_TARGET)
        string(APPEND CMAKE_${language}_FLAGS_INIT
               "--target=${CMAKE_${language}_COMPILER_TARGET} ")
    endif()
endforeach()

#
# Suppress warnings about duplicate input files. This one is CMake's fault -
# linked libraries are added more than once if they are depended on by multiple
# targets.
#

string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT "--diag_suppress L6304W ")

#
# The Arm Compiler 6 support in CMake, until hopefully, v3.18.3, incorrectly
# uses the Clang-style -Xlinker linker wrapper flag, even though its linker is
# configured to locate and use 'armlink' instead of 'armclang'. This is just a
# bit of a hacky workaround to reset it to nothing whenever the CMake toolchain
# logic tries to override it.
#
# https://gitlab.kitware.com/cmake/cmake/-/issues/21154#note_826110
#

if(CMAKE_VERSION VERSION_LESS "3.18.3")
    # cmake-lint: disable=C0111

    macro(scp_reset_linker_wrapper_flag)
        set(CMAKE_C_LINKER_WRAPPER_FLAG "")
        set(CMAKE_CXX_LINKER_WRAPPER_FLAG "")
    endmacro()

    variable_watch(CMAKE_C_LINKER_WRAPPER_FLAG scp_reset_linker_wrapper_flag)
    variable_watch(CMAKE_CXX_LINKER_WRAPPER_FLAG scp_reset_linker_wrapper_flag)
endif()
