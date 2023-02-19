#
# Arm SCP/MCP Software
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# cmake-lint: disable=C0301

#
# Set up the framework target.
#
# This stage does the basic framework target configuration, including adding any
# default source files, setting up include directories and adding any
# preprocessor definitions.
#

add_library(framework)

target_include_directories(framework
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(
    framework
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/assert.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_arch.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_dlist.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_id.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_interrupt.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_io.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_log.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_mm.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_module.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_ring.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_slist.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_status.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_string.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_delayed_resp.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_time.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/stdlib.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_core.c")


if(SCP_ENABLE_SUB_SYSTEM_MODE)
    target_compile_definitions(framework PUBLIC "BUILD_HAS_SUB_SYSTEM_MODE")
endif()

if(SCP_ENABLE_NOTIFICATIONS)
    target_sources(framework
                   PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_notification.c")

    target_compile_definitions(framework PUBLIC "BUILD_HAS_NOTIFICATION")
endif()

if(SCP_ENABLE_SCMI_NOTIFICATIONS)
    target_compile_definitions(framework PUBLIC "BUILD_HAS_SCMI_NOTIFICATIONS")
    if(SCP_ENABLE_SCMI_SENSOR_EVENTS)
        target_compile_definitions(framework PUBLIC "BUILD_HAS_SCMI_SENSOR_EVENTS")
    endif()
endif()

if(SCP_ENABLE_SCMI_PERF_FAST_CHANNELS)
    target_compile_definitions(framework PUBLIC "BUILD_HAS_FAST_CHANNELS")
    target_compile_definitions(framework PUBLIC "BUILD_HAS_SCMI_PERF_FAST_CHANNELS")
endif()

if(SCP_ENABLE_PLUGIN_HANDLER)
    target_compile_definitions(framework PUBLIC "BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(framework PUBLIC BUILD_MODE_DEBUG)
endif()

if(SCP_ENABLE_CLOCK_TREE_MGMT)
    target_compile_definitions(framework PUBLIC "BUILD_HAS_CLOCK_TREE_MGMT")
endif()

if(SCP_ENABLE_SCMI_SENSOR_V2)
    target_compile_definitions(framework PUBLIC "BUILD_HAS_SCMI_SENSOR_V2")
    target_compile_definitions(framework PUBLIC "BUILD_HAS_SENSOR_TIMESTAMP")
    target_compile_definitions(framework PUBLIC "BUILD_HAS_SENSOR_MULTI_AXIS")
    target_compile_definitions(framework PUBLIC "BUILD_HAS_SENSOR_EXT_ATTRIBS")
    target_compile_definitions(framework PUBLIC "BUILD_HAS_SENSOR_SIGNED_VALUE")
endif()

if(SCP_ENABLE_SENSOR_TIMESTAMP)
    target_compile_definitions(framework PUBLIC "BUILD_HAS_SENSOR_TIMESTAMP")
endif()

if(SCP_ENABLE_SENSOR_MULTI_AXIS)
    target_compile_definitions(framework PUBLIC "BUILD_HAS_SENSOR_MULTI_AXIS")
endif()

if(SCP_ENABLE_SENSOR_EXT_ATTRIBS)
    target_compile_definitions(framework PUBLIC "BUILD_HAS_SENSOR_EXT_ATTRIBS")
endif()

if(SCP_ENABLE_SENSOR_SIGNED_VALUE)
    target_compile_definitions(framework PUBLIC "BUILD_HAS_SENSOR_SIGNED_VALUE")
endif()

if(SCP_ENABLE_INBAND_MSG_SUPPORT)
    target_compile_definitions(framework PUBLIC "BUILD_HAS_INBAND_MSG_SUPPORT")
endif()

if(SCP_ENABLE_ATU_MANAGE)
    target_compile_definitions(framework PUBLIC "BUILD_HAS_ATU_MANAGE")
endif()

if(SCP_ENABLE_ATU_DELEGATE)
    target_compile_definitions(framework PUBLIC "BUILD_HAS_ATU_DELEGATE")
endif()

#
# Handle the framework logging filter level.
#

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SCP_LOG_LEVEL_DEFAULT "INFO")
else()
    set(SCP_LOG_LEVEL_DEFAULT "WARN")
endif()

if(NOT DEFINED SCP_LOG_LEVEL)
    set(SCP_LOG_LEVEL
        "${SCP_LOG_LEVEL_DEFAULT}"
        CACHE STRING "Minimum logging level.")

    set_property(CACHE SCP_LOG_LEVEL PROPERTY STRINGS "DEBUG" "INFO" "WARN"
                                                      "ERROR" "CRIT" "DISABLED")
endif()

target_compile_definitions(framework
                           PUBLIC FWK_LOG_LEVEL=FWK_LOG_LEVEL_${SCP_LOG_LEVEL})

#
# Generate `<fwk_module_idx.h>` and `<fwk_module_list.c>` based on the list of
# modules we have been given. While we're at it, we add these modules as
# dependencies of the framework.
#

list(LENGTH SCP_MODULES SCP_MODULE_IDX_MAX)

# cmake-lint: disable=E1120

foreach(idx RANGE ${SCP_MODULE_IDX_MAX})
    if(idx EQUAL SCP_MODULE_IDX_MAX)
        string(APPEND SCP_MODULE_IDX_GEN "    FWK_MODULE_IDX_COUNT = ${idx},\n")

        break()
    endif()

    list(GET SCP_MODULES ${idx} SCP_MODULE)

    string(MAKE_C_IDENTIFIER ${SCP_MODULE} SCP_MODULE)
    string(TOUPPER ${SCP_MODULE} SCP_MODULE_UPPER)

    # cmake-format: off

    string(APPEND SCP_MODULE_IDX_GEN "    FWK_MODULE_IDX_${SCP_MODULE_UPPER} = ${idx},\n")
    string(APPEND SCP_MODULE_ID_INIT_GEN "#define FWK_MODULE_ID_${SCP_MODULE_UPPER}_INIT FWK_ID_MODULE_INIT(FWK_MODULE_IDX_${SCP_MODULE_UPPER})\n")
    string(APPEND SCP_MODULE_ID_GEN "#define FWK_MODULE_ID_${SCP_MODULE_UPPER} FWK_ID_MODULE(FWK_MODULE_IDX_${SCP_MODULE_UPPER})\n")
    string(APPEND SCP_MODULE_ID_CONST_GEN "static const fwk_id_t fwk_module_id_${SCP_MODULE} = FWK_MODULE_ID_${SCP_MODULE_UPPER}_INIT;\n")

    string(APPEND SCP_MODULE_EXTERN_GEN "extern const struct fwk_module module_${SCP_MODULE};\n")
    string(APPEND SCP_MODULE_EXTERN_CONFIG_GEN "extern const struct fwk_module_config config_${SCP_MODULE};\n")
    string(APPEND SCP_MODULE_GEN "    &module_${SCP_MODULE},\n")
    string(APPEND SCP_MODULE_CONFIG_GEN "    &config_${SCP_MODULE},\n")

    # cmake-format: on

    #
    # Create the `BUILD_HAS_MOD_<X>` definition.
    #

    target_compile_definitions(framework
                               PUBLIC "BUILD_HAS_MOD_${SCP_MODULE_UPPER}=1")
endforeach()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/fwk_module_idx.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/include/fwk_module_idx.h")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/fwk_module_list.c.in"
               "${CMAKE_CURRENT_BINARY_DIR}/src/fwk_module_list.c")

target_include_directories(framework
                           PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/include")

target_sources(framework
               PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/src/fwk_module_list.c")

#
# Build framework version string
#
# The framework version string uses Git to describe the current commit or tag.
# If Git errors out, or if the project is not part of a Git repository, the
# version defaults to `vX.Y.Z-<unknown>`.
#

find_package(Git)

if(GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" describe --tags --dirty --always
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        OUTPUT_VARIABLE SCP_DESCRIBE
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if(NOT SCP_DESCRIBE)
    set(SCP_DESCRIBE "v${SCP_VERSION}-<unknown>")
endif()

if (NOT DEFINED BUILD_VERSION_DESCRIBE_STRING)
    set(BUILD_VERSION_DESCRIBE_STRING ${SCP_DESCRIBE})
endif()
if (NOT (DEFINED BUILD_VERSION_MAJOR AND DEFINED BUILD_VERSION_MINOR AND DEFINED BUILD_VERSION_PATCH))
    set(BUILD_VERSION_MAJOR ${SCP_VERSION_MAJOR})
    set(BUILD_VERSION_MINOR ${SCP_VERSION_MINOR})
    set(BUILD_VERSION_PATCH ${SCP_VERSION_PATCH})
endif()

target_compile_definitions(
    framework
    PUBLIC "BUILD_VERSION_DESCRIBE_STRING=\"${BUILD_VERSION_DESCRIBE_STRING}\""
           "BUILD_VERSION_MAJOR=${BUILD_VERSION_MAJOR}"
           "BUILD_VERSION_MINOR=${BUILD_VERSION_MINOR}"
           "BUILD_VERSION_PATCH=${BUILD_VERSION_PATCH}")

#
# Pull in any include directories explicitly exposed by the firmware. We need
# these for certain `<fmw_<x>.h>` header files.
#

target_include_directories(
    framework
    PRIVATE
        $<TARGET_PROPERTY:${SCP_FIRMWARE_TARGET},INTERFACE_INCLUDE_DIRECTORIES>)

#
# Make sure the framework links privately to all of the modules, as it depends
# on some of the symbols that they export (namely `module_<x>`).
#

foreach(target IN LISTS SCP_MODULE_TARGETS)
    target_link_libraries(framework PRIVATE ${target})
endforeach()

#
# If the debugger has been requested, we also need to link to that explicitly,
# as the debugger is initially set up through the framework.
#

if(SCP_ENABLE_DEBUGGER)
    target_link_libraries(framework PUBLIC debugger)
endif()

target_link_libraries(framework INTERFACE ${SCP_ARCHITECTURE_TARGET})
