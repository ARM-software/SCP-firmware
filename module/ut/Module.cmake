#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(SCP_MODULE "ut")
set(SCP_MODULE_TARGET "module-ut")

if(SCP_ENABLE_UT)
    list(APPEND SCP_MODULES "ut")
endif()
