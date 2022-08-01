#
# Arm SCP/MCP Software
# Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include_guard()

include(SCPPreprocessSource)

# .rst:
#
# .. command:: scp_module_trace
#
# Checks if SCP_TRACE_ENABLE_MOD_<module name>` is set and enables `FWK_TRACE`
#
# .. cmake:: scp_module_trace(<module_name>)
#
macro(scp_module_trace module_name)

    # Creates a module name tag moving to upper case and replacing `-` by `_`.
    string(TOUPPER ${module_name} SCP_MODULE_TAG)
    string(REPLACE "-" "_" SCP_MODULE_TAG ${SCP_MODULE_TAG})
    set(MOD_TRACE_ENABLE_FLAG "SCP_TRACE_ENABLE_MOD_${SCP_MODULE_TAG}")

    # Checks if the enable flag is set.
    if(${MOD_TRACE_ENABLE_FLAG})
        add_compile_definitions(PUBLIC FWK_TRACE_ENABLE)
    endif()
endmacro()
