#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

list(APPEND SCP_ARCHITECTURE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/arm/arm-m")
list(APPEND SCP_ARCHITECTURE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/arm/armv8-a")
list(APPEND SCP_ARCHITECTURE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/none/host")
list(APPEND SCP_ARCHITECTURE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/none/optee")

set(SCP_ARCHITECTURE_ "${SCP_ARCHITECTURE}")

foreach(path IN LISTS SCP_ARCHITECTURE_PATHS)
    unset(SCP_ARCHITECTURE)

    include("${path}/Architecture.cmake" OPTIONAL
            RESULT_VARIABLE SCP_ARCHITECTURE_LIST_FILE)

    if(NOT SCP_ARCHITECTURE_LIST_FILE)
        # cmake-format: off
        message(WARNING
            "No architecture list file found!\n"

            "${path}/Architecture.cmake\n"

            "This architecture path was provided as part of a firmware "
            "initial-cache file ('Firmware.cmake'), but its "
            "'Architecture.cmake' file could not be located.")
        # cmake-format: on

        continue()
    elseif(NOT DEFINED SCP_ARCHITECTURE)
        # cmake-format: off
        message(WARNING
            "No name given by architecture metadata file!\n"

            "${path}/Architecture.cmake\n"

            "This architecture metadata file has not yielded an architecture "
            "name and as such cannot be identified. please ensure that you have "
            "set `SCP_ARCHITECTURE` in your 'Architecture.cmake' file.")
        # cmake-format: on

        continue()
    elseif(NOT DEFINED SCP_ARCHITECTURE_TARGET)
        # cmake-format: off
        message(WARNING
            "No target given by architecture: ${SCP_ARCHITECTURE}\n"

            "This architecture has not yielded a target name, and as such "
            "cannot be used as a dependency for any other targets. "
            "please ensure that you have set `SCP_ARCHITECTURE_TARGET` in your "
            "'Architecture.cmake' file.")
        # cmake-format: on

        continue()
    endif()

    #
    # Append this architecture to the list of valid architectures, and make sure
    # we track its target and source directory.
    #

    list(APPEND SCP_VALID_ARCHITECTURES "${SCP_ARCHITECTURE}")
    list(APPEND SCP_VALID_ARCHITECTURE_TARGETS "${SCP_ARCHITECTURE_TARGET}")
    list(APPEND SCP_VALID_ARCHITECTURE_SOURCE_DIRS "${path}")
endforeach()

set(SCP_ARCHITECTURE "${SCP_ARCHITECTURE_}")

if(NOT "${SCP_ARCHITECTURE}" IN_LIST SCP_VALID_ARCHITECTURES)
    # cmake-format: off
    message(FATAL_ERROR
        "No architecture found: ${SCP_ARCHITECTURE}\n"

        "The architecture requested by the firmware you are trying to build "
        "has not been found. This is probably because the firmware "
        "initial-cache file you provided has requested a custom architecture "
        "without providing its path.")
    # cmake-format: on
endif()

#
# Load in the architecture support library target.
#

list(FIND SCP_VALID_ARCHITECTURES "${SCP_ARCHITECTURE}" SCP_ARCHITECTURE_IDX)
list(GET SCP_VALID_ARCHITECTURE_TARGETS ${SCP_ARCHITECTURE_IDX}
     SCP_ARCHITECTURE_TARGET)
list(GET SCP_VALID_ARCHITECTURE_SOURCE_DIRS ${SCP_ARCHITECTURE_IDX}
     SCP_ARCHITECTURE_SOURCE_DIR)

set(SCP_ARCHITECTURE_BINARY_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/${SCP_ARCHITECTURE}")

add_subdirectory("${SCP_ARCHITECTURE_SOURCE_DIR}"
                 "${SCP_ARCHITECTURE_BINARY_DIR}" EXCLUDE_FROM_ALL)

#
# Ensure the firmware links to the architecture library, and that the
# architecture library links to the framework. It's the firmware dependency on
# the architecture library that officially causes the the program entrypoint to
# be linked.
#

target_link_libraries(${SCP_FIRMWARE_TARGET} PRIVATE ${SCP_ARCHITECTURE_TARGET})
target_link_libraries(${SCP_ARCHITECTURE_TARGET} PRIVATE framework)

target_include_directories(
    ${SCP_ARCHITECTURE_TARGET}
    PRIVATE
        $<TARGET_PROPERTY:${SCP_FIRMWARE_TARGET},INTERFACE_INCLUDE_DIRECTORIES>)

target_compile_definitions(
    ${SCP_ARCHITECTURE_TARGET}
    PRIVATE
        $<TARGET_PROPERTY:${SCP_FIRMWARE_TARGET},INTERFACE_COMPILE_DEFINITIONS>)

set(SCP_ARCHITECTURE_TARGET
    "${SCP_ARCHITECTURE_TARGET}"
    PARENT_SCOPE)
