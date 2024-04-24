#
# Arm SCP/MCP Software
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include("${CMAKE_CURRENT_LIST_DIR}/Clang-Base.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/Generic-Baremetal.cmake")

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    execute_process(
        COMMAND "which" "${SCP_LLVM_SYSROOT_CC}"
        OUTPUT_VARIABLE LLVM_SYSROOT_CC_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(
        COMMAND "dirname" "${LLVM_SYSROOT_CC_PATH}"
        OUTPUT_VARIABLE LLVM_SYSROOT_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(APPEND LLVM_SYSROOT_PATH "/../aarch64-none-elf")
    cmake_path(NORMAL_PATH LLVM_SYSROOT_PATH OUTPUT_VARIABLE LLVM_SYSROOT_PATH)
    set(gcc-multi-dir "")
else()
    execute_process(
        COMMAND "${SCP_LLVM_SYSROOT_CC}" "-print-sysroot"
        OUTPUT_VARIABLE LLVM_SYSROOT_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(
        COMMAND "${SCP_LLVM_SYSROOT_CC}" "-print-multi-directory"
                "-mcpu=${CMAKE_SYSTEM_PROCESSOR}" "-mthumb"
        OUTPUT_VARIABLE gcc-multi-dir
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

foreach(language IN ITEMS ASM C CXX)

    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
        string(APPEND CMAKE_${language}_FLAGS_INIT
               "-mstrict-align -fno-builtin -DAARCH64 -D__ASSEMBLY__ ")
        string(APPEND CMAKE_${language}_FLAGS_INIT
               "-I\"${LLVM_SYSROOT_PATH}/include\" ")
    endif()

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "cortex-m(3|7|33|55|85)")
        set(BUILD_TARGET "-mcpu=${CMAKE_SYSTEM_PROCESSOR} -mthumb ")
        string(APPEND CMAKE_${language}_FLAGS_INIT "${BUILD_TARGET}")
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "cortex-m7")
            set(CLANG_BUILTINS_ARCH "armv7em")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "cortex-m33")
            set(CLANG_BUILTINS_ARCH "armv8m.main")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "cortex-m3")
            set(CLANG_BUILTINS_ARCH "armv7m")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "cortex-m(55|85)")
            set(CLANG_BUILTINS_ARCH "armv8.1m.main")
        endif()
    endif()

    string(APPEND CMAKE_${language}_FLAGS_INIT
           "--target=${CMAKE_${language}_COMPILER_TARGET} ")

    string(APPEND CMAKE_${language}_FLAGS_INIT
           "--sysroot=${LLVM_SYSROOT_PATH} ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-ffunction-sections ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-fdata-sections ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-fshort-enums ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-Wall -Werror -Wextra ")
    string(APPEND CMAKE_${language}_FLAGS_INIT
           "-Wno-error=deprecated-declarations ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-Wno-unused-parameter ")
    string(APPEND CMAKE_${language}_FLAGS_INIT
           "-Wno-missing-field-initializers ")

    string(APPEND CMAKE_${language}_FLAGS_DEBUG_INIT "-Og")
endforeach()

string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--gc-sections ")
string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT "-I\"${LLVM_SYSROOT_PATH}/include\" ")

string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT
       "-L${LLVM_SYSROOT_PATH}/lib/${gcc-multi-dir} ")
if(NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld ")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT
           "-nostdlib -lclang_rt.builtins-${CLANG_BUILTINS_ARCH} ")
    foreach(crtobj IN ITEMS crti crtbegin crt0 crtend crtn)
        execute_process(
            COMMAND "${SCP_LLVM_SYSROOT_CC}" "--print-file-name=${crtobj}.o"
                    "-mcpu=${CMAKE_SYSTEM_PROCESSOR}" "-mthumb"
            OUTPUT_VARIABLE gcc-${crtobj}
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        cmake_path(NORMAL_PATH gcc-${crtobj} OUTPUT_VARIABLE gcc-${crtobj})
        string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT "${gcc-${crtobj}} ")
    endforeach()
endif()
