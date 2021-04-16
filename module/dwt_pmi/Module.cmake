#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(SCP_MODULE "dwt-pmi")
set(SCP_MODULE_TARGET "module-dwt-pmi")

if(SCP_ENABLE_PMI)
    list(APPEND SCP_MODULES "dwt-pmi")
endif()
