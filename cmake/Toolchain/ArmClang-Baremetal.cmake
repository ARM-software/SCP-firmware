#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include("${CMAKE_CURRENT_LIST_DIR}/ArmClang-Base.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/Generic-Baremetal.cmake")

foreach(language IN ITEMS ASM C CXX)
    string(APPEND CMAKE_${language}_FLAGS_INIT "-ffunction-sections ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-fdata-sections ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-fshort-enums ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-mfloat-abi=soft ")

    string(APPEND CMAKE_${language}_FLAGS_DEBUG_INIT "-Og ")
endforeach()

string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT "--remove ")
string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT "--fpu=SoftVFP ")
