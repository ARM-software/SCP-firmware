#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#


# Function to help set dependencies on .dts files
# (copied from zephyr project)
#
# 'toolchain_parse_make_rule' is a function that parses the output of
# 'gcc -M'.
#
# The argument 'input_file' is in input parameter with the path to the
# file with the dependency information.
#
# The argument 'include_files' is an output parameter with the result
# of parsing the include files.
function(toolchain_parse_make_rule input_file include_files)
  file(READ ${input_file} input)

  # The file is formatted like this:
  # empty_file.o: misc/empty_file.c \
  # nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts \
  # nrf52840_qiaa.dtsi

  # Get rid of the backslashes
  string(REPLACE "\\" ";" input_as_list ${input})

  # Pop the first line and treat it specially
  list(GET input_as_list 0 first_input_line)
  string(FIND ${first_input_line} ": " index)
  math(EXPR j "${index} + 2")
  string(SUBSTRING ${first_input_line} ${j} -1 first_include_file)
  list(REMOVE_AT input_as_list 0)

  list(APPEND result ${first_include_file})

  # Add the other lines
  list(APPEND result ${input_as_list})

  # Strip away the newlines and whitespaces
  list(TRANSFORM result STRIP)

  set(${include_files} ${result} PARENT_SCOPE)
endfunction()

# Use .dts files to configure modules if selected in project.
#
# device tree bindings are used to populate configuration tables in the firmware
#

# set up the generator tool 
set(DTS_GEN_SCRIPT ${CMAKE_SOURCE_DIR}/tools/dts/dt_gen_config.py)
set(PYTHON_EXEC python3)
set(PYTHON_DTLIB_SRC_PATH ${CMAKE_SOURCE_DIR}/contrib/python-devicetree/src)
if ($ENV{PYTHONPATH})
  set(ENV{PYTHONPATH} $ENV{PYTHONPATH}:${PYTHON_DTLIB_SRC_PATH})
else()
  set(ENV{PYTHONPATH} ${PYTHON_DTLIB_SRC_PATH})
endif()

message(STATUS "env PYTHONPATH = $ENV{PYTHONPATH}")

# output working directory for dts elements
set(DTS_BIN_DIR ${CMAKE_BINARY_DIR}/dts)
file(MAKE_DIRECTORY ${DTS_BIN_DIR})
set(DTS_HEADER_DIR ${CMAKE_BINARY_DIR}/framework/include)
file(MAKE_DIRECTORY ${DTS_HEADER_DIR})

# set up source directories used by common .dts / .dtsi files
set(DTS_SRC_COMMON ${CMAKE_SOURCE_DIR}/dts/src)
set(DTS_SRC_COMMON_INC ${CMAKE_SOURCE_DIR}/dts/include)

set(DTS_CPP_TEMP_OUT ${DTS_BIN_DIR}/${SCP_FIRMWARE}.dts.pre.tmp)
set(DTS_CPP_DEPS_OUT ${DTS_BIN_DIR}/${SCP_FIRMWARE}.dts.pre.d)
set(DTS_EDT_PICKLE_OUT   ${DTS_BIN_DIR}/edt.pickle)
set(DTS_FWK_HEADER_OUT ${DTS_HEADER_DIR}/fwk_dt_config.h)
set(DTS_VENDOR_PREFIXES ${CMAKE_SOURCE_DIR}/dts/bindings/vendor-prefixes.txt)
set(DTS_GEN_DTS_DBG_OUT ${DTS_BIN_DIR}/${SCP_FIRMWARE}.dts.out)
set(DTS_BINDINGS_DIRS ${CMAKE_SOURCE_DIR}/dts/bindings) 

message(STATUS "compiling devicetree files: ${SCP_FIRMWARE_DTS_SOURCE} using ${DTS_GEN_SCRIPT}")

# add framework includes for common bindings - fixed & generated.
list(APPEND SCP_FIRMWARE_DTS_INCLUDE "${CMAKE_SOURCE_DIR}/framework/include")
list(APPEND SCP_FIRMWARE_DTS_INCLUDE "${CMAKE_BINARY_DIR}/framework/include")

# list of dts include source dirs
list(APPEND
    DTS_INC
    ${DTS_SRC_COMMON}
    ${DTS_SRC_COMMON_INC}
    ${SCP_FIRMWARE_DTS_INCLUDE}
    )
message(STATUS "DTS_INC dirs ${DTS_INC}")


# need to call preprocessor and generator if the product has a .dts file
if(SCP_FIRMWARE_DTS_SOURCE)
    # C Pre-Processor on dts & dtsi to create output temp file
    # create include command line flags for CPP 
    unset(DTS_INC_FLAG_CPP)
    set(DTS_INC_FLAG_CPP
        -include ${SCP_FIRMWARE_DTS_SOURCE}
        )
    message(STATUS "DTS inc ${DTS_INC_FLAG_CPP}")

    # create system include dir flags for CPP
    unset(DTS_SYSTEM_INC_DIR_FLAG_CPP)
    foreach(dts_inc_dir ${DTS_INC})
        get_filename_component(full_path ${dts_inc_dir} REALPATH)
        list(APPEND
            DTS_SYSTEM_INC_DIR_FLAG_CPP
            -isystem ${full_path}
            )
    endforeach()
    message(STATUS "DTS INC flags : ${DTS_SYSTEM_INC_DIR_FLAG_CPP}")
    execute_process(
        COMMAND  ${CMAKE_C_COMPILER}
        -x assembler-with-cpp
        -nostdinc
        -undef
        -D DTS_COMPILE
        ${DTS_SYSTEM_INC_DIR_FLAG_CPP} # dts include dirs as -isystem <path>  options
        ${DTS_INC_FLAG_CPP}            # dts source as -include <source> options
        -P
        -E   # stop after pre-processing
        -MD  # generate deps file
        -MF  ${DTS_CPP_DEPS_OUT}
        -o  ${DTS_CPP_TEMP_OUT}
        ${CMAKE_SOURCE_DIR}/dts/misc/empty_file.c 
        WORKING_DIRECTORY ${DTS_BIN_DIR}
        RESULT_VARIABLE ret
        )
    if(NOT "${ret}" STREQUAL "0")
           message(FATAL_ERROR "C pre-process command failed with return code: ${ret}")
    endif()
    
    # run generator on preprocessed temp file.
    set(CMD_GEN_CFG ${PYTHON_EXEC} ${DTS_GEN_SCRIPT}
        --dts ${DTS_CPP_TEMP_OUT}
        --scp-base-dir ${CMAKE_SOURCE_DIR}
        --header-out ${DTS_FWK_HEADER_OUT}
        --dts-out ${DTS_GEN_DTS_DBG_OUT} # As a debugging aid
        --bindings-dirs ${DTS_BINDINGS_DIRS}
        --edt-pickle-out ${DTS_EDT_PICKLE_OUT}
        --vendor-prefixes ${DTS_VENDOR_PREFIXES}
        --dtc-flags "none"
        --verbose 1
    )
    message(STATUS "command used: ${CMD_GEN_CFG} raw3 in ${DTS_BIN_DIR}")  
    execute_process(
        COMMAND ${CMD_GEN_CFG}
        WORKING_DIRECTORY ${DTS_BIN_DIR}
        COMMAND_ERROR_IS_FATAL ANY
    )

    # add dependencies from processed .dts file
    # Parse the generated dependency file to find the DT sources that
    # were included and then add them to the list of files that trigger
    # a re-run of CMake.
    toolchain_parse_make_rule(
        ${DTS_CPP_DEPS_OUT}
        include_files # Output parameter
        )

    set_property(DIRECTORY APPEND PROPERTY
        CMAKE_CONFIGURE_DEPENDS
        ${include_files}
        ${DTS_GEN_SCRIPT}
        )

endif()
