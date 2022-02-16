#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_sources(${SCP_MODULE_TARGET}
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_morello_sensor.c")

if(SCP_MORELLO_SENSOR_LIB_PATH)
    add_library(morello_sensor_driver STATIC IMPORTED GLOBAL)

    set_target_properties(
        morello_sensor_driver
        PROPERTIES IMPORTED_LOCATION "${SCP_MORELLO_SENSOR_LIB_PATH}")
else()
    target_sources(
        ${SCP_MODULE_TARGET}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/morello_sensor_driver.c")
endif()

target_include_directories(
    ${SCP_MODULE_TARGET}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(
    ${SCP_MODULE_TARGET} PRIVATE module-timer module-power-domain module-sensor
                                 module-morello-scp2pcc)

if(SCP_MORELLO_SENSOR_LIB_PATH)
    target_link_libraries(${SCP_MODULE_TARGET} PRIVATE morello_sensor_driver)
endif()
