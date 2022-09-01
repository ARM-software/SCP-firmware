#
# Arm SCP/MCP Software
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(${SCP_MODULE_TARGET}
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_juno_xrp7724.c")

target_compile_definitions(${SCP_MODULE_TARGET} PUBLIC -DJUNO_VARIANT_BOARD=0)
target_compile_definitions(${SCP_MODULE_TARGET} PUBLIC -DJUNO_VARIANT_FVP=1)

if (SCP_PLATFORM_VARIANT STREQUAL "BOARD")
    target_compile_definitions(${SCP_MODULE_TARGET}
        PUBLIC -DPLATFORM_VARIANT=JUNO_VARIANT_BOARD)
elseif (SCP_PLATFORM_VARIANT STREQUAL "FVP")
    target_compile_definitions(${SCP_MODULE_TARGET}
        PUBLIC -DPLATFORM_VARIANT=JUNO_VARIANT_FVP)
endif()

target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-i2c)
target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-psu)
target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-sensor)
target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-timer)
