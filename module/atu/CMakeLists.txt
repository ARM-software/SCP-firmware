#
# Arm SCP/MCP Software
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(
    ${SCP_MODULE_TARGET}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(
    ${SCP_MODULE_TARGET}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_atu.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/atu_common.c")

# Throw an error if neither of the CMake flags are configured
if ("atu" IN_LIST SCP_MODULES)
    if(NOT(SCP_ENABLE_ATU_MANAGE) AND NOT(SCP_ENABLE_ATU_DELEGATE))
    # cmake-format: off
        message(FATAL_ERROR
            "\nInvalid ATU driver configuration!\nAtleast"
            " one of the following CMake flags must be set to TRUE:\n"
            " - SCP_ENABLE_ATU_MANAGE\n"
            " - SCP_ENABLE_ATU_DELEGATE\n")
    # cmake-format: on
    endif()
endif()

if(SCP_ENABLE_ATU_MANAGE)
    # The firmware has access to the ATU
    # cmake-format: off
    message(WARNING
        "\nATU access enabled in ${SCP_FIRMWARE} firmware! "
        "The ${SCP_FIRMWARE} firmware can configure the ATU to access system "
        "memory with increased privileges.")
    # cmake-format: on
    target_sources(
        ${SCP_MODULE_TARGET}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/atu_manage_utils.c"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/atu_manage.c")
endif()

if(SCP_ENABLE_ATU_DELEGATE)
    # The firmware does not have access to configure the ATU
    target_sources(
        ${SCP_MODULE_TARGET}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/atu_delegate_utils.c"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/atu_delegate.c")

    target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-transport
        module-timer)
endif()
