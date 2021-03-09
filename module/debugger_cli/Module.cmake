#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(SCP_MODULE "debugger-cli")
set(SCP_MODULE_TARGET "module-debugger-cli")

if(SCP_ENABLE_DEBUGGER)
    list(APPEND SCP_MODULES "debugger-cli")
endif()
