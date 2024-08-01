#
# Arm SCP/MCP Software
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# cmake-lint: disable=C0301

cmake_minimum_required(VERSION 3.18.3)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

#
# Individual firmware targets are configured based on the 'Firmware.cmake'
# living in the firmware source directory that the user provides to us, which
# determines the default set of configuration options we are given, including
# the toolchain file.
#

if(SCP_FIRMWARE_SOURCE_DIR)
    get_filename_component(SCP_FIRMWARE_SOURCE_DIR "${SCP_FIRMWARE_SOURCE_DIR}"
                           ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}/product")

    include("${SCP_FIRMWARE_SOURCE_DIR}/Firmware.cmake")

    if((NOT SCP_FIRMWARE) OR (NOT SCP_FIRMWARE_TARGET))
        # cmake-format: off
        message(FATAL_ERROR
            "Insufficient firmware metadata provided.\n"

            "Please ensure your 'Firmware.cmake' has set both `SCP_FIRMWARE` "
            "and `SCP_FIRMWARE_TARGET` to the name of your firmware and the "
            "firmware's CMake target respectively.")
        # cmake-format: on
    endif()

    if(NOT SCP_FIRMWARE_BINARY_DIR)
        #
        # Derive the binary directory from the location of the source directory
        # if it's in-tree.
        #

        file(RELATIVE_PATH SCP_FIRMWARE_BINARY_DIR "${CMAKE_SOURCE_DIR}"
             "${SCP_FIRMWARE_SOURCE_DIR}")
    endif()

    if(SCP_FIRMWARE_BINARY_DIR MATCHES "\\.\\.")
        # cmake-format: off
        message(FATAL_ERROR
            "Invalid firmware binary directory.\n"

            "Please ensure your firmware binary directory is a relative path. "
            "This path is used as the location within the project binary "
            "directory that will be used for firmware artifacts.")
        # cmake-format: on
    endif()
endif()

#
# Handle automatic selection of the toolchain. This only occurs if the user has
# not explicitly provided the path to a toolchain file and the firmware has
# given us a default preference.
#

if(SCP_TOOLCHAIN_INIT AND (NOT CMAKE_TOOLCHAIN_FILE))
    #
    # Let the firmware decide what its default toolchain should be, but allow
    # the user to override it.
    #

    if(NOT SCP_TOOLCHAIN)
        set(SCP_TOOLCHAIN "${SCP_TOOLCHAIN_INIT}")
    endif()

    set(CMAKE_TOOLCHAIN_FILE
        "${SCP_FIRMWARE_SOURCE_DIR}/Toolchain-${SCP_TOOLCHAIN}.cmake")
endif()

if(SCP_TOOLCHAIN STREQUAL "Clang" AND NOT SCP_LLVM_SYSROOT_CC)
    message(
        FATAL_ERROR
            "When Clang is set as toolchain SCP_LLVM_SYSROOT_CC must be defined"
    )
endif()

project(
    SCP
    VERSION 2.14.0
    DESCRIPTION "Arm SCP/MCP Software"
    HOMEPAGE_URL
        "https://developer.arm.com/tools-and-software/open-source-software/firmware/scp-firmware"
    LANGUAGES C ASM)

#
# Configure the default build type to be "Release". We choose to default to a
# release build as non-developer consumers will generally want a release build,
# whereas developers who need a debug build are generally familiar with how to
# enable it in the build system.
#

if((NOT CMAKE_BUILD_TYPE) AND (NOT CMAKE_CONFIGURATION_TYPES))
    set(CMAKE_BUILD_TYPE
        "Release"
        CACHE STRING "Build type." FORCE)

    set_property(
        CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel"
                                                "RelWithDebInfo")
endif()

#
# Set the default C standard to C11 plus any compiler extensions.
#

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_EXTENSIONS TRUE)

#
# We use `__declspec` when available, but Clang needs to explicitly enable
# support for it before we can use them.
#

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    string(APPEND CMAKE_C_FLAGS " -fms-extensions")
endif()

#
# Enable inter-procedural optimization for all targets if we can.
# `CheckIPOSupported` doesn't play particularly nicely with toolchains that
# cannot build executables right off the bat, so we skip this check if we're
# cross compiling.
#

include(CheckIPOSupported)
include(CMakeDependentOption)

check_ipo_supported(RESULT SCP_IPO_SUPPORTED)

cmake_dependent_option(
    SCP_ENABLE_IPO
    "Enable the interprocedural optimization (IPO) if supported?"
    "${SCP_ENABLE_IPO_INIT}" "DEFINED SCP_ENABLE_IPO_INIT" "${SCP_ENABLE_IPO}")

if(CMAKE_CROSSCOMPILING
   AND (CMAKE_C_COMPILER_ID STREQUAL "GNU")
   AND SCP_ENABLE_IPO)
    set(SCP_IPO_SUPPORTED TRUE)
endif()

cmake_dependent_option(
    CMAKE_INTERPROCEDURAL_OPTIMIZATION "Enable interprocedural optimization?"
    TRUE "SCP_IPO_SUPPORTED;SCP_ENABLE_IPO" FALSE)

#
# Export a `compile_commands.json` file for generators that support it.
#

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

#
# Set up Cppcheck if it's available and enabled.
#
if(NOT DISABLE_CPPCHECK)
    find_program(CMAKE_C_CPPCHECK NAMES "cppcheck")

    if(NOT CMAKE_C_CPPCHECK)
        unset(CMAKE_C_CPPCHECK CACHE)
    endif()

    if(CMAKE_C_CPPCHECK)
        # Prepend cppcheck_wrapper.py before binary to perform pre and post
        # cppcheck operations.
        set(CPPCHECK_WRAPPER ${CMAKE_SOURCE_DIR}/tools/cppcheck_wrapper.py)
        # cmake-format: off

        list(PREPEND CMAKE_C_CPPCHECK  ${CPPCHECK_WRAPPER})
        list(APPEND CMAKE_C_CPPCHECK "--quiet")
        list(APPEND CMAKE_C_CPPCHECK "--suppressions-list=${CMAKE_CURRENT_SOURCE_DIR}/tools/cppcheck_suppress_list.txt")
        list(APPEND CMAKE_C_CPPCHECK "--enable=all")
        list(APPEND CMAKE_C_CPPCHECK "--error-exitcode=1")

        if(SCP_ARCHITECTURE STREQUAL "arm-m")
            if(CMAKE_SYSTEM_PROCESSOR MATCHES "cortex-m(33|55|85)")
                list(APPEND CMAKE_C_CPPCHECK "--library=${CMAKE_CURRENT_SOURCE_DIR}/.armv8m.cppcheck.cfg")
            elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "cortex-m(3|7)")
                list(APPEND CMAKE_C_CPPCHECK "--library=${CMAKE_CURRENT_SOURCE_DIR}/.armv7m.cppcheck.cfg")
            endif()
        endif()

        if(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
            list(APPEND CMAKE_C_CPPCHECK "--library=${CMAKE_CURRENT_SOURCE_DIR}/.armclang.cppcheck.cfg")
        else()
            list(APPEND CMAKE_C_CPPCHECK "--library=${CMAKE_CURRENT_SOURCE_DIR}/.gnu.cppcheck.cfg")
        endif()

        if(COMMAND_OUTPUT_VERBOSE)
             list(APPEND CMAKE_C_CPPCHECK "verbose")
        endif()

        # cmake-format: on
    endif()
endif()

#
# Set up Clang-Tidy if it's available and enabled.
#
if(ENABLE_CLANG_TIDY)

    find_program(
        CMAKE_C_CLANG_TIDY
        NAMES "clang-tidy"
              "clang-tidy-3.6"
              "clang-tidy-3.7"
              "clang-tidy-3.8"
              "clang-tidy-3.9"
              "clang-tidy-4.0"
              "clang-tidy-5.0"
              "clang-tidy-6.0"
              "clang-tidy-7"
              "clang-tidy-8"
              "clang-tidy-9"
              "clang-tidy-10"
              "clang-tidy-11")

    if(NOT CMAKE_C_CLANG_TIDY)
        unset(CMAKE_C_CLANG_TIDY CACHE)
    endif()

    if(CMAKE_C_CLANG_TIDY)
        # cmake-format: off

        if(CMAKE_C_COMPILER_TARGET)
            list(APPEND CMAKE_C_CLANG_TIDY "--extra-arg=--target=${CMAKE_C_COMPILER_TARGET}")
        endif()

        foreach(dir IN LISTS CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES)
            list(APPEND CMAKE_C_CLANG_TIDY "--extra-arg=-isystem")
            list(APPEND CMAKE_C_CLANG_TIDY "--extra-arg=${dir}")
            list(APPEND CMAKE_C_CLANG_TIDY "--extra-arg=-fms-extensions")
        endforeach()

        if(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
            list(APPEND CMAKE_C_CLANG_TIDY "--extra-arg=-D__ARM_PROMISE=__builtin_assume")
            list(APPEND CMAKE_C_CLANG_TIDY "--extra-arg=-D__ARMCC_VERSION=600000")
            list(APPEND CMAKE_C_CLANG_TIDY "--extra-arg=-D__ESCAPE__(x)=(x)")
        endif()

        list(APPEND CMAKE_C_CLANG_TIDY "--quiet")

        # cmake-format: on
    endif()

endif()

#
# Set up Include What You Use ("IWYU") if it's available and enabled.
#
if(ENABLE_IWYU)

    find_program(CMAKE_C_INCLUDE_WHAT_YOU_USE NAMES iwyu include-what-you-use)

    if(NOT CMAKE_C_INCLUDE_WHAT_YOU_USE)
        unset(CMAKE_C_INCLUDE_WHAT_YOU_USE CACHE)
    endif()

    if(CMAKE_C_INCLUDE_WHAT_YOU_USE)
        # cmake-format: off

        if(CMAKE_C_COMPILER_TARGET)
            list(APPEND CMAKE_C_INCLUDE_WHAT_YOU_USE "--target=${CMAKE_C_COMPILER_TARGET}")
        endif()

        foreach(dir IN LISTS CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES)
            list(APPEND CMAKE_C_INCLUDE_WHAT_YOU_USE "-isystem")
            list(APPEND CMAKE_C_INCLUDE_WHAT_YOU_USE "${dir}")
        endforeach()

        if(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
            list(APPEND CMAKE_C_INCLUDE_WHAT_YOU_USE "-D__ARM_PROMISE=__builtin_assume")
            list(APPEND CMAKE_C_INCLUDE_WHAT_YOU_USE "-D__ARMCC_VERSION=600000")
            list(APPEND CMAKE_C_INCLUDE_WHAT_YOU_USE "-D__ESCAPE__(x)=(x)")
        endif()

        # cmake-format: on
    endif()

endif()

#
# Try to locate Arm Compiler's 'fromelf' tool.
#

if(CMAKE_C_COMPILER_ID MATCHES "ARMClang")
    get_filename_component(base ${CMAKE_C_COMPILER} DIRECTORY)

    find_program(SCP_FROMELF fromelf ${base})

    if(SCP_FROMELF)
        mark_as_advanced(SCP_FROMELF)
    endif()
endif()

#
# Try to identify any standard libraries that require special treatment.
#

include(CheckSymbolExists)

check_symbol_exists("__NEWLIB__" "sys/features.h" SCP_HAVE_NEWLIB)

#
# If the user asks for it, we can save them some time by creating a flat binary
# from the firmware's compiled ELF image.
#

include(CMakeDependentOption)

cmake_dependent_option(
    SCP_GENERATE_FLAT_BINARY "Generate a flat binary (.bin) image?"
    "${SCP_GENERATE_FLAT_BINARY_INIT}" "DEFINED SCP_GENERATE_FLAT_BINARY_INIT"
    "${SCP_GENERATE_FLAT_BINARY}")

#
# If the firmware developer has given us initial values for these configuration
# options, we can expose them to the user.
#

include(CMakeDependentOption)

# Common build options

if(SCP_ARCHITECTURE STREQUAL "optee")
    set(SCP_ENABLE_SUB_SYSTEM_MODE_INIT TRUE)
endif()

set(SCP_ENABLE_OVERRIDE_FIRMWARE_NAME
    ${SCP_ENABLE_OVERRIDE_FIRMWARE_NAME_INIT}
    CACHE STRING "Override firmware binary name")

cmake_dependent_option(
    SCP_ENABLE_SUB_SYSTEM_MODE "Enable the execution as a sub-system?"
    "${SCP_ENABLE_SUB_SYSTEM_MODE_INIT}"
    "DEFINED SCP_ENABLE_SUB_SYSTEM_MODE_INIT"
    "${SCP_ENABLE_SUB_SYSTEM_MODE}")

cmake_dependent_option(
    SCP_ENABLE_NOTIFICATIONS "Enable the notification subsystem?"
    "${SCP_ENABLE_NOTIFICATIONS_INIT}" "DEFINED SCP_ENABLE_NOTIFICATIONS_INIT"
    "${SCP_ENABLE_NOTIFICATIONS}")

cmake_dependent_option(
    SCP_ENABLE_OUTBAND_MSG_SUPPORT
    "Enable Transport Out Band Message Support?"
    "${SCP_ENABLE_OUTBAND_MSG_SUPPORT_INIT}"
    "DEFINED SCP_ENABLE_OUTBAND_MSG_SUPPORT_INIT"
    "${SCP_ENABLE_OUTBAND_MSG_SUPPORT}")

cmake_dependent_option(
    SCP_ENABLE_RESOURCE_PERMISSIONS
    "Enable the resource permission support?"
    "${SCP_ENABLE_RESOURCE_PERMISSIONS_INIT}"
    "DEFINED SCP_ENABLE_RESOURCE_PERMISSIONS_INIT"
    "${SCP_ENABLE_RESOURCE_PERMISSIONS}")

cmake_dependent_option(
    SCP_ENABLE_SCMI_NOTIFICATIONS
    "Enable the SCMI notifications?"
    "${SCP_ENABLE_SCMI_NOTIFICATIONS_INIT}"
    "DEFINED SCP_ENABLE_SCMI_NOTIFICATIONS_INIT"
    "${SCP_ENABLE_SCMI_NOTIFICATIONS}")

cmake_dependent_option(
    SCP_ENABLE_SCMI_SENSOR_EVENTS
    "Enable the SCMI sensor events?"
    "${SCP_ENABLE_SCMI_SENSOR_EVENTS_INIT}"
    "DEFINED SCP_ENABLE_SCMI_SENSOR_EVENTS_INIT"
    "${SCP_ENABLE_SCMI_SENSOR_EVENTS}")

cmake_dependent_option(
    SCP_ENABLE_FAST_CHANNELS
    "Enable the transport Fast Channels?"
    "${SCP_ENABLE_FAST_CHANNELS_INIT}"
    "DEFINED SCP_ENABLE_FAST_CHANNELS_INIT"
    "${SCP_ENABLE_FAST_CHANNELS}")

# Include firmware specific build options
include("${SCP_FIRMWARE_SOURCE_DIR}/Buildoptions.cmake" OPTIONAL)

#
# Wrap `add_executable` in a way that allows us to do some extra processing on
# the firmware target.(e.g. flat binary generation) This is necessary almost
# exclusively because  `add_custom_command` cannot be used with a target created
# in a different directory.
#

# cmake-lint: disable=C0103,C0111

macro(add_executable target)
    _add_executable(${target} ${ARGN})

    if("${target}" STREQUAL "${SCP_FIRMWARE_TARGET}")
        if(SCP_GENERATE_FLAT_BINARY)
            #
            # Invoke 'objcopy' or 'fromelf', which we use to generate a flat
            # binary for the generated firmware image. We make this optional
            # because some binaries consist of disjoint sections far apart from
            # one another, which can generate huge binaries.
            #

            if(SCP_FROMELF)
                # cmake-format: off
                add_custom_command(
                    TARGET ${target} POST_BUILD
                    COMMAND ${SCP_FROMELF}
                    ARGS
                        --bin "$<TARGET_FILE:${target}>"
                        --output "$<TARGET_FILE_DIR:${target}>/$<TARGET_PROPERTY:${target},OUTPUT_NAME>.bin"
                    COMMENT "Generating flat binary ${target}.bin")
                # cmake-format: on
            elseif(CMAKE_OBJCOPY)
                # cmake-format: off
                add_custom_command(
                    TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_OBJCOPY}
                    ARGS -O binary
                        "$<TARGET_FILE:${target}>"
                        "$<TARGET_FILE_DIR:${target}>/$<TARGET_FILE_BASE_NAME:${target}>.bin"
                    COMMENT "Generating flat binary ${target}.bin")
                # cmake-format: on
            endif()
        endif()
    endif()
endmacro()

if(SCP_FIRMWARE)
    #
    # Pull in the CMSIS package.
    #

    add_subdirectory("contrib/cmsis" EXCLUDE_FROM_ALL)

    #
    # Load in the firmware. We do this now so that the firmware list files can
    # make adjustments to any modules it wishes to load in, configure the
    # architecture library, etc.
    #

    add_subdirectory("${SCP_FIRMWARE_SOURCE_DIR}" "${SCP_FIRMWARE_BINARY_DIR}"
                     EXCLUDE_FROM_ALL)

    #
    # Generate a map file for the firmware.
    #

    if(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
        target_link_options(
            ${SCP_FIRMWARE_TARGET}
            PRIVATE "LINKER:--map"
                    "LINKER:--list=$<TARGET_FILE_DIR:${SCP_FIRMWARE_TARGET}>/$<TARGET_FILE_BASE_NAME:${SCP_FIRMWARE_TARGET}>.map")
    else()
        target_link_options(
            ${SCP_FIRMWARE_TARGET}
            PRIVATE "LINKER:--cref"
                    "LINKER:-Map=$<TARGET_FILE_DIR:${SCP_FIRMWARE_TARGET}>/$<TARGET_FILE_BASE_NAME:${SCP_FIRMWARE_TARGET}>.map")
    endif()

    #
    # Make the firmware target a part of the 'all' pseudo-target and give it a
    # fixed output binary name so that different toolchains don't generate
    # different names (which is desirable especially for higher-level build
    # systems).
    #

    if("${SCP_ENABLE_OVERRIDE_FIRMWARE_NAME}" STREQUAL "")
        set(SCP_FIRMWARE_NAME ${SCP_FIRMWARE_TARGET})
    else()
        set(SCP_FIRMWARE_NAME ${SCP_ENABLE_OVERRIDE_FIRMWARE_NAME})
    endif()

    set_target_properties(
        ${SCP_FIRMWARE_TARGET}
        PROPERTIES EXCLUDE_FROM_ALL FALSE
                   OUTPUT_NAME "${SCP_FIRMWARE_NAME}"
                   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

    set_target_properties(
        ${SCP_FIRMWARE_TARGET}
        PROPERTIES EXCLUDE_FROM_ALL FALSE
        SUFFIX ".elf")
    #
    # Load in the CLI debugger if it was requested.
    #

    cmake_dependent_option(
        SCP_ENABLE_DEBUGGER "Enable the debugger-cli subsystem?"
        "${SCP_ENABLE_DEBUGGER_INIT}" "DEFINED SCP_ENABLE_DEBUGGER_INIT"
        "${SCP_ENABLE_DEBUGGER}")

    if(SCP_ENABLE_DEBUGGER)
        add_subdirectory("debugger" EXCLUDE_FROM_ALL)
    endif()

    #
    # Load in the architecture support library.
    #

    add_subdirectory("arch" EXCLUDE_FROM_ALL)

    #
    # Load in the modules specified, plus any that need to be included to
    # support other modules.
    #

    add_subdirectory("module" EXCLUDE_FROM_ALL)

    #
    # Load in the framework. This should be the last of the targets we need.
    #

    add_subdirectory("framework" EXCLUDE_FROM_ALL)

    #
    # Collect up all the source files used to build the various targets created
    # above.
    #

    # cmake-format: off

    string(APPEND scp_sources "$<TARGET_PROPERTY:framework,INTERFACE_SOURCES>$<SEMICOLON>")
    string(APPEND scp_sources "$<TARGET_PROPERTY:framework,SOURCES>$<SEMICOLON>")
    string(APPEND scp_sources "$<TARGET_PROPERTY:${SCP_ARCHITECTURE_TARGET},INTERFACE_SOURCES>$<SEMICOLON>")
    string(APPEND scp_sources "$<TARGET_PROPERTY:${SCP_ARCHITECTURE_TARGET},SOURCES>$<SEMICOLON>")
    string(APPEND scp_sources "$<TARGET_PROPERTY:${SCP_FIRMWARE_TARGET},INTERFACE_SOURCES>$<SEMICOLON>")
    string(APPEND scp_sources "$<TARGET_PROPERTY:${SCP_FIRMWARE_TARGET},SOURCES>$<SEMICOLON>")

    string(APPEND scp_includes "$<TARGET_PROPERTY:framework,INTERFACE_INCLUDE_DIRECTORIES>$<SEMICOLON>")
    string(APPEND scp_includes "$<TARGET_PROPERTY:framework,INCLUDE_DIRECTORIES>$<SEMICOLON>")
    string(APPEND scp_includes "$<TARGET_PROPERTY:${SCP_ARCHITECTURE_TARGET},INTERFACE_INCLUDE_DIRECTORIES>$<SEMICOLON>")
    string(APPEND scp_includes "$<TARGET_PROPERTY:${SCP_ARCHITECTURE_TARGET},INCLUDE_DIRECTORIES>$<SEMICOLON>")
    string(APPEND scp_includes "$<TARGET_PROPERTY:${SCP_FIRMWARE_TARGET},INTERFACE_INCLUDE_DIRECTORIES>$<SEMICOLON>")
    string(APPEND scp_includes "$<TARGET_PROPERTY:${SCP_FIRMWARE_TARGET},INCLUDE_DIRECTORIES>$<SEMICOLON>")

    string(APPEND scp_defines "$<TARGET_PROPERTY:framework,INTERFACE_COMPILE_DEFINITIONS>$<SEMICOLON>")
    string(APPEND scp_defines "$<TARGET_PROPERTY:framework,COMPILE_DEFINITIONS>$<SEMICOLON>")
    string(APPEND scp_defines "$<TARGET_PROPERTY:${SCP_ARCHITECTURE_TARGET},INTERFACE_COMPILE_DEFINITIONS>$<SEMICOLON>")
    string(APPEND scp_defines "$<TARGET_PROPERTY:${SCP_ARCHITECTURE_TARGET},COMPILE_DEFINITIONS>$<SEMICOLON>" )
    string(APPEND scp_defines "$<TARGET_PROPERTY:${SCP_FIRMWARE_TARGET},INTERFACE_COMPILE_DEFINITIONS>$<SEMICOLON>")
    string(APPEND scp_defines "$<TARGET_PROPERTY:${SCP_FIRMWARE_TARGET},COMPILE_DEFINITIONS>$<SEMICOLON>")

    list(APPEND scp_libraries "$<TARGET_FILE:framework>")
    list(APPEND scp_libraries "$<TARGET_FILE:${SCP_ARCHITECTURE_TARGET}>")
    list(APPEND scp_libraries "$<TARGET_FILE:${SCP_FIRMWARE_TARGET}>")

    foreach(module IN LISTS SCP_MODULE_TARGETS)
        string(APPEND scp_sources "$<TARGET_PROPERTY:${module},INTERFACE_SOURCES>$<SEMICOLON>")
        string(APPEND scp_sources "$<TARGET_PROPERTY:${module},SOURCES>$<SEMICOLON>")

        string(APPEND scp_includes "$<TARGET_PROPERTY:${module},INTERFACE_INCLUDE_DIRECTORIES>$<SEMICOLON>")
        string(APPEND scp_includes "$<TARGET_PROPERTY:${module},INCLUDE_DIRECTORIES>$<SEMICOLON>")

        string(APPEND scp_defines "$<TARGET_PROPERTY:${module},INTERFACE_COMPILE_DEFINITIONS>$<SEMICOLON>")
        string(APPEND scp_defines "$<TARGET_PROPERTY:${module},COMPILE_DEFINITIONS>$<SEMICOLON>")

        list(APPEND scp_libraries "$<TARGET_FILE:${module}>")
    endforeach()

    set(scp_sources $<REMOVE_DUPLICATES:${scp_sources}>)
    set(scp_includes $<REMOVE_DUPLICATES:${scp_includes}>)
    set(scp_defines $<REMOVE_DUPLICATES:${scp_defines}>)

    # cmake-format: on

    #
    # Load in the documentation generation.
    #
    add_subdirectory("doc" EXCLUDE_FROM_ALL)
endif()


set(SCP_FIRMWARE_LIB "${SCP_FIRMWARE_TARGET}-all")

# merged libraries is only built when `make ${SCP_FIRMWARE_LIB}` is issued
add_custom_target(${SCP_FIRMWARE_LIB}
    # COMMAND/COMMENT must be upper case
        COMMAND ${CMAKE_AR} rcT $<TARGET_FILE_DIR:${SCP_FIRMWARE_TARGET}>/lib${SCP_FIRMWARE_LIB}.a ${scp_libraries}
        COMMENT " Merging all libraries in a single lib${SCP_FIRMWARE_LIB}.a"
)

add_dependencies(${SCP_FIRMWARE_LIB} ${SCP_FIRMWARE_TARGET})

#
# Set up global inclusions and exclusions for source file quality assurance
# tools. This is intended to filter in external directories (e.g. out-of-tree
# modules) and filter out third-party directories.
#

list(APPEND glob_includes "${SCP_SOURCE_DIR}")

list(APPEND glob_excludes "^${SCP_SOURCE_DIR}/contrib/cmsis/git")
list(APPEND glob_excludes "^${SCP_SOURCE_DIR}/contrib/run-clang-format/git")

list(APPEND cmake_globs "CMakeLists.txt")
list(APPEND cmake_globs "*.cmake")

list(APPEND yaml_globs "*.yml")
list(APPEND yaml_globs "*.yaml")

list(APPEND c_globs "*.[ch]")
list(APPEND c_globs "*.[ch]pp")
list(APPEND c_globs "*.[ch]xx")

#
# Glob sources and place them in their respective variables. Globs for source
# types can be with specified with a `${type}_globs` variable, which should be a
# list of globs to be fed into `file(GLOB_RECURSE ...)`.
#

# cmake-lint: disable=C0103

foreach(type cmake yaml c)
    unset(sources)

    foreach(include IN LISTS glob_includes)
        foreach(glob IN LISTS ${type}_globs)
            file(GLOB_RECURSE _sources "${include}/${glob}")
            list(APPEND sources ${_sources})
        endforeach()
    endforeach()

    foreach(exclude IN LISTS glob_excludes)
        list(FILTER sources EXCLUDE REGEX "${exclude}")
    endforeach()

    set(${type}_sources ${sources})
endforeach()

#
# Configure cmake-format.
#

find_package(CMakeFormat OPTIONAL_COMPONENTS Format Lint)

if(TARGET CMakeFormat::Format)
    add_custom_target(
        format-cmake
        COMMAND CMakeFormat::Format -i "${cmake_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Formatting CMake sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND format_targets "format-cmake")

    add_custom_target(
        check-cmake
        COMMAND CMakeFormat::Format --check "${cmake_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Checking CMake sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND check_targets "check-cmake")
endif()

if(TARGET CMakeFormat::Lint)
    add_custom_target(
        lint-cmake
        COMMAND CMakeFormat::Lint --suppress-decorations "${cmake_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Linting CMake sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND lint_targets "lint-cmake")
endif()

#
# Configure yaml-format.
#

find_package(Python3 COMPONENTS Interpreter)

if(Python3_Interpreter_FOUND)
    add_custom_target(
        format-yaml
        COMMAND Python3::Interpreter "${SCP_SOURCE_DIR}/tools/yaml-format.py"
                format -i "${yaml_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Formatting YAML sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND format_targets "format-yaml")

    add_custom_target(
        check-yaml
        COMMAND Python3::Interpreter "${SCP_SOURCE_DIR}/tools/yaml-format.py"
                diff --check "${yaml_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Checking YAML sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND check_targets "check-yaml")
endif()

#
# Configure yamllint.
#

find_package(Yamllint)

if(Yamllint_FOUND)
    add_custom_target(
        lint-yaml
        COMMAND Yamllint -s "${yaml_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Linting YAML sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND lint_targets "lint-yaml")
endif()

#
# Configure Clang-Format.
#

find_package(Clang OPTIONAL_COMPONENTS Format FormatGit)

if(Clang_FormatGit_FOUND)
    add_custom_target(
        format-diff-c
        COMMAND Clang::FormatGit HEAD
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Formatting modified C/C++ sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND format_diff_targets "format-diff-c")
endif()

if(Clang_Format_FOUND)
    add_custom_target(
        format-c
        COMMAND Clang::Format -i "${c_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Formatting C/C++ sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND format_targets "format-c")
endif()

#
# Configure run-clang-format.
#

find_package(Python3 COMPONENTS Interpreter)

if(Python3_Interpreter_FOUND)
    add_custom_target(
        check-c
        COMMAND
            Python3::Interpreter
            "${SCP_SOURCE_DIR}/contrib/run-clang-format/git/run-clang-format.py"
            --clang-format-executable "${Clang_Format_EXECUTABLE}"
            "${c_sources}"
        WORKING_DIRECTORY "${SCP_SOURCE_DIR}"
        COMMENT "Checking C sources..."
        COMMAND_EXPAND_LISTS)

    list(APPEND check_targets "check-c")
endif()

#
# Create the final check targets. These targets consist of miscellaneous quality
# assurance tasks like linting and formatting, and act as dummy targets that
# invoke the various tools we set up above.
#

add_custom_target(
    format-diff
    DEPENDS ${format_diff_targets}
    COMMENT "Formatting modified sources...")

add_custom_target(
    format
    DEPENDS ${format_targets}
    COMMENT "Formatting all sources...")

add_custom_target(
    lint
    DEPENDS ${lint_targets}
    COMMENT "Linting all sources...")

add_custom_target(
    check
    DEPENDS ${check_targets} lint
    COMMENT "Checking lint...")
