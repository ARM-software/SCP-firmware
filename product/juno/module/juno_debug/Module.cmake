#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(SCP_MODULE "juno-debug")

set(SCP_MODULE_TARGET "module-juno-debug")

if(SCP_ENABLE_DEBUG_UNIT)
    list(APPEND SCP_MODULES "juno-debug")
endif()