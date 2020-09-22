#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include_guard()

# cmake-lint: disable=C0113,C0301

# .rst:
#
# .. command:: scp_preprocess_source
#
# Preprocess a file with the C preprocessor.
#
# .. cmake:: scp_preprocess_source(<target> <source> <output>)
#
# This macro creates a target ``<target>`` which preprocesses a source file
# ``<source>``, giving the file ``<output>``.
#
# You can set properties on the target created by this macro through the
# standard means. If you wish you to add extra compile definitions or include
# directories, you can do so by manually adding them to the
# ``INCLUDE_DIRECTORIES`` and ``COMPILE_DEFINITIONS`` target properties.
# ``target_include_directories`` and friends cannot be used on the target
# created by this macro.
macro(scp_preprocess_source target source output)
    #
    # CMake provides the `CMAKE_C_CREATE_PREPROCESSED_SOURCE` variable, which
    # describes the command line required to preprocess a C source file. This
    # variable is in a format similar to this:
    #
    # <CMAKE_C_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -E <SOURCE> >
    # <PREPROCESSED_SOURCE>
    #
    # We do some processing on this variable to convert these bracket-surrounded
    # names to variables we set. For example, `<DEFINES>` is replaced with
    # `${cpp_DEFINES}`. We then need to do some string replacement magic to
    # expand that string out to the value of the actual variable.
    #
    # The values for some of these, namely include directories, definitions and
    # other compiler options, come from properties set on the target by the
    # caller. These are typically taken from the target that this preprocessed
    # source file.
    #

    set(command ${CMAKE_C_CREATE_PREPROCESSED_SOURCE})
    string(REPLACE " " ";" command ${command})

    string(TOUPPER ${CMAKE_BUILD_TYPE} config)

    set(cpp_SOURCE "${source}")
    set(cpp_PREPROCESSED_SOURCE "${output}")

    set(cpp_CMAKE_C_COMPILER "${CMAKE_C_COMPILER}")

    unset(cpp_DEFINES)
    unset(cpp_INCLUDES)

    # cmake-format: off

    separate_arguments(cpp_FLAGS
        UNIX_COMMAND "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${config}} -P")

    list(APPEND cpp_FLAGS "$<TARGET_PROPERTY:${target},COMPILE_OPTIONS>")
    list(APPEND cpp_DEFINES "$<TARGET_PROPERTY:${target},COMPILE_DEFINITIONS>")
    list(APPEND cpp_INCLUDES "$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>")

    set(cpp_DEFINES "$<$<BOOL:${cpp_DEFINES}>:-D$<JOIN:${cpp_DEFINES},$<SEMICOLON>-D>>")
    set(cpp_INCLUDES "$<$<BOOL:${cpp_INCLUDES}>:-I$<JOIN:${cpp_INCLUDES},$<SEMICOLON>-I>>")

    # cmake-format: on

    string(REGEX REPLACE "<([[A-Z_]+)>" "\${cpp_\\1}" command "${command}")
    string(REGEX MATCH "\\\${[^}]*}" match "${command}")

    while(match)
        string(REGEX REPLACE "\\\${(.*)}" "\\1" variable "${match}")
        string(REPLACE "\${${variable}}" "${${variable}}" command "${command}")
        string(REGEX MATCH "\\\${[^}]*}" match "${command}")
    endwhile()

    add_custom_command(
        OUTPUT ${output}
        MAIN_DEPENDENCY ${source}
        COMMAND "${command}"
        VERBATIM COMMAND_EXPAND_LISTS)

    add_custom_target(${target} DEPENDS ${output})
endmacro()
