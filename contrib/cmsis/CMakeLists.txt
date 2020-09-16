#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.18.3)

project(
    CMSIS
    VERSION 5.2.0
    DESCRIPTION "Cortex Microcontroller Software Interface Standard (CMSIS)"
    HOMEPAGE_URL "https://developer.arm.com/tools-and-software/embedded/cmsis"
    LANGUAGES C)

add_library(rtos2 INTERFACE)

target_include_directories(
    rtos2 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/git/CMSIS/RTOS2/Include")

add_library(cmsis::rtos2 ALIAS rtos2)
