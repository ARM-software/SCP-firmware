#
# Arm SCP/MCP Software
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include("${CMAKE_CURRENT_LIST_DIR}/GNU-Base.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/Generic-Baremetal.cmake")

foreach(language IN ITEMS ASM C CXX)

    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
        if(DEFINED SCP_AARCH64_PROCESSOR_TARGET)
            string(APPEND CMAKE_${language}_FLAGS_INIT
               "-mcpu=${SCP_AARCH64_PROCESSOR_TARGET} ")
        endif()
        string(APPEND CMAKE_${language}_FLAGS_INIT
            "-mstrict-align -fno-builtin -DAARCH64 -D__ASSEMBLY__ ")
    endif()

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "cortex-m(0|0plus|3|7|33|55)")
        string(APPEND CMAKE_${language}_FLAGS_INIT "-mthumb ")
        string(APPEND CMAKE_${language}_FLAGS_INIT
            "-mcpu=${CMAKE_SYSTEM_PROCESSOR} ")
    endif()

    string(APPEND CMAKE_${language}_FLAGS_INIT "-ffunction-sections ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-fdata-sections ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-fshort-enums ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-Wall -Werror -Wextra ")
    string(APPEND CMAKE_${language}_FLAGS_INIT
           "-Wno-error=deprecated-declarations ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-Wno-unused-parameter ")
    string(APPEND CMAKE_${language}_FLAGS_INIT
           "-Wno-missing-field-initializers ")

    string(APPEND CMAKE_${language}_FLAGS_DEBUG_INIT "-Og")
endforeach()

string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--gc-sections ")
