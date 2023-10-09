#
# Arm SCP/MCP Software
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# cmake-lint: disable=C0301

#
# Handle the module libraries.
#
# The list of modules and any additional module paths are given by the firmware
# initial-cache file through `SCP_MODULES` and `SCP_MODULE_PATHS`. Here we
# iterate through each of the module paths we've been given and load a
# 'Module.cmake' file from within, which describes the module. The module paths
# are listed in an alphabetical order.
#

list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/amu_mmap")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/atu")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/apcontext")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/armv7m_mpu")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/armv8m_mpu")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/bootloader")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/clock")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/cmn600")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/cmn_skeena")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/css_clock")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ddr_phy500")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/debug")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/debugger_cli")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/dmc500")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/dmc620")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/dvfs")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/dw_apb_i2c")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/gtimer")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/i2c")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/isys_rom")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/mhu")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/mhu2")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/mock_clock")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/mock_ppu")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/mock_psu")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/mock_voltage_domain")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/mpmm")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/msg_smt")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/msys_rom")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/optee/clock")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/optee/console")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/optee/mbx")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/optee/reset")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/optee/smt")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/optee/voltd_regulator")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/pcid")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/pik_clock")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/pl011")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/power_domain")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ppu_v0")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ppu_v1")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/psu")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/reg_sensor")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/reset_domain")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/resource_perms")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/scmi")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/scmi_apcore")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/scmi_clock")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/scmi_perf")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/scmi_power_domain")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/scmi_reset_domain")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/scmi_sensor")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/scmi_sensor_req")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/scmi_system_power")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/scmi_system_power_req")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/scmi_voltage_domain")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/sds")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/sensor")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/sensor_smcf_drv")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/sid")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/smcf")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/amu_smcf_drv")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/sp805")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ssc")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/statistics")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/stdio")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/system_info")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/system_pll")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/system_power")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/thermal_mgmt")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/timer")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/traffic_cop")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/transport")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/voltage_domain")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/xr77128")
list(APPEND SCP_MODULE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/fch_polled")

#
# Remove paths if this variable is defined by the platform target's
# Firmware.cmake
#
if(SCP_MODULE_EXCLUDE_PATHS)
    list(REMOVE_ITEM SCP_MODULE_PATHS ${SCP_MODULE_EXCLUDE_PATHS})
endif()

foreach(source_dir IN LISTS SCP_MODULE_PATHS)
    unset(SCP_MODULE)

    include("${source_dir}/Module.cmake" OPTIONAL
            RESULT_VARIABLE SCP_MODULE_LIST_FILE)

    if(NOT SCP_MODULE_LIST_FILE)
        # cmake-format: off

        message(FATAL_ERROR
            "No module list file found!\n"

            "${source_dir}/Module.cmake\n"

            "This module path was provided as part of a firmware initial-cache "
            "file ('Firmware.cmake'), but its 'Module.cmake' file could not be "
            "located.")

        # cmake-format: on
    elseif(NOT DEFINED SCP_MODULE)
        # cmake-format: off

        message(FATAL_ERROR
            "No module name given for module!\n"

            "${source_dir}/Module.cmake\n"

            "This module metadata file has not yielded a module name and as "
            "such cannot be identified. "
            "please ensure that you have set `SCP_MODULE` in your "
            "'Module.cmake' file.")

        # cmake-format: on
    endif()

    #
    # Append this module to the list of valid modules and track its source
    # directory.
    #

    list(APPEND SCP_VALID_MODULES "${SCP_MODULE}")
    list(APPEND SCP_VALID_MODULE_TARGETS "${SCP_MODULE_TARGET}")
    list(APPEND SCP_VALID_MODULE_SOURCE_DIRS "${source_dir}")
endforeach()

#
# Load in each module, one by one.
#
# We've stored a list of modules that we've located inside `SCP_VALID_MODULES`.
# Now, we need to run through the list of modules that the firmware requested
# and ensure that we found it. If we did, load it into the build system. If we
# didn't, exit and let the user know.
#

set(SCP_MISSING_MODULES "${SCP_MODULES}")
list(REMOVE_ITEM SCP_MISSING_MODULES ${SCP_VALID_MODULES})

foreach(module IN LISTS SCP_MISSING_MODULES)
    # cmake-format: off
    message(WARNING
        "Module requested, but not found: ${module}\n"

        "The firmware requested this module, but we were unable to locate it. "
        "If you are the firmware developer, please ensure that you have added "
        "the module path to `SCP_MODULE_PATHS` in your 'Firmware.cmake' file.")
    # cmake-format: on
endforeach()

if(SCP_MISSING_MODULES)
    message(FATAL_ERROR "Missing modules were requested!")
endif()

#
# Wrap the standard target creation/manipulation facilities so that we can
# control the underlying target type and its sources when the module creates its
# target.
#

include(CMakeParseArguments)

# cmake-lint: disable=C0103,C0111

macro(add_library target)
    set(args "${ARGN}")

    set(is_module FALSE)

    if("SCP_MODULE" IN_LIST args)
        if(SCP_MODULE_HEADER_ONLY)
            #
            # If this module is being loaded in as a header-only module, then we
            # silently replace any `add_library` calls using the `SCP_MODULE`
            # type with `INTERFACE`.
            #

            string(REPLACE "SCP_MODULE" "INTERFACE" args "${args}")
        else()
            string(REPLACE "SCP_MODULE" "" args "${args}")
        endif()

        set(is_module TRUE)
    endif()

    _add_library(${target} ${args})

    if(is_module)
        set_target_properties("${target}" PROPERTIES _SCP_IS_MODULE TRUE)
    endif()
endmacro()

# cmake-lint: disable=C0103,C0111

function(scp_generate_builtin_module_wrapper builtin)
    function(${builtin} target)
        set(args "${ARGN}")

        get_target_property(is_module "${target}" _SCP_IS_MODULE)
        get_target_property(type "${target}" TYPE)
        get_target_property(is_imported "${target}" IMPORTED)

        set(is_interface FALSE)

        if(type STREQUAL "INTERFACE_LIBRARY")
            set(is_interface TRUE)
        endif()

        if(is_imported)
            set(is_interface TRUE)
        endif()

        if(is_module AND is_interface)
            #
            # If this module is being pulled in as an interface library target,
            # we need to strip any `PRIVATE` arguments and convert any `PUBLIC`
            # ones to `INTERFACE`.
            #

            cmake_parse_arguments("scp_${CMAKE_CURRENT_FUNCTION}" "" ""
                                  "PRIVATE" ${ARGN})

            set(args "${scp_${CMAKE_CURRENT_FUNCTION}_UNPARSED_ARGUMENTS}")
            string(REPLACE "PUBLIC" "INTERFACE" args "${args}")
        endif()

        if(args)
            cmake_language(CALL _${CMAKE_CURRENT_FUNCTION} ${target} ${args})
        endif()
    endfunction()
endfunction()

scp_generate_builtin_module_wrapper(target_include_directories)
scp_generate_builtin_module_wrapper(target_sources)
scp_generate_builtin_module_wrapper(target_link_libraries)
scp_generate_builtin_module_wrapper(target_link_options)

#
# Load in all the modules we have access to.
#

# cmake-lint: disable=C0103

foreach(SCP_MODULE IN LISTS SCP_VALID_MODULES)
    #
    # We have the module, so grab its target and the source directory, both of
    # which were specified by its 'Module.cmake' file.
    #

    list(FIND SCP_VALID_MODULES "${SCP_MODULE}" SCP_MODULE_IDX)
    list(GET SCP_VALID_MODULE_TARGETS ${SCP_MODULE_IDX} SCP_MODULE_TARGET)
    list(GET SCP_VALID_MODULE_SOURCE_DIRS ${SCP_MODULE_IDX}
         SCP_MODULE_SOURCE_DIR)

    #
    # Pull the module in, creating the target. Modules that we do not intend on
    # linking to are created as interface targets, so that other modules can use
    # their headers if they so wish.
    #

    if(SCP_MODULE IN_LIST SCP_MODULES)
        set(SCP_MODULE_HEADER_ONLY FALSE)
        set(SCP_MODULE_PUBLIC "PUBLIC")
    else()
        set(SCP_MODULE_HEADER_ONLY TRUE)
        set(SCP_MODULE_PUBLIC "INTERFACE")
    endif()

    add_subdirectory(
        "${SCP_MODULE_SOURCE_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}/modules/${SCP_MODULE}" EXCLUDE_FROM_ALL)

    #
    # Link the firmware to the module, and the module to the framework.
    #

    target_link_libraries(${SCP_FIRMWARE_TARGET} PRIVATE ${SCP_MODULE_TARGET})
    target_link_libraries(${SCP_MODULE_TARGET} ${SCP_MODULE_PUBLIC} framework)

    #
    # Handle any modules that we actually intend on linking to.
    #

    if(SCP_MODULE IN_LIST SCP_MODULES)
        #
        # Expose any firmware interface headers to the module.
        #

        target_include_directories(
            ${SCP_MODULE_TARGET}
            PRIVATE
                $<TARGET_PROPERTY:${SCP_FIRMWARE_TARGET},INTERFACE_INCLUDE_DIRECTORIES>
        )

        #
        # Make sure this module is linked.
        #

        list(APPEND SCP_MODULE_TARGETS "${SCP_MODULE_TARGET}")
    endif()
endforeach()

#
# Export the updated module list and their targets so they can be used from the
# root variable scope downwards.
#

set(SCP_MODULES
    "${SCP_MODULES}"
    PARENT_SCOPE)
set(SCP_MODULE_TARGETS
    "${SCP_MODULE_TARGETS}"
    PARENT_SCOPE)
