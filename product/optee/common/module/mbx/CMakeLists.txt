#
# Arm SCP/MCP Software
# Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# These include directories are relative to SCP-firmware submodule position in
# optee-os tree
# Those includes are needed for mutex defifitnion that is used in optee_smt
# notification
target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${SCP_OPTEE_DIR}/core/include/"
                                  "${SCP_OPTEE_DIR}/lib/libutils/ext/include/"
                                  "${SCP_OPTEE_DIR}/lib/libutee/include/")

target_link_libraries(${SCP_MODULE_TARGET}
                      PRIVATE module-optee-smt
                              module-msg-smt
                              module-scmi)

target_sources(${SCP_MODULE_TARGET}
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_optee_mbx.c")
