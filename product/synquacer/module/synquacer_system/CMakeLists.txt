#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(
    ${SCP_MODULE_TARGET}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(
    ${SCP_MODULE_TARGET}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/nic400.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/mmu500.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_synquacer_system.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/synquacer_main.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/thermal_sensor.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/transaction_sw.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/synquacer_pd_manage.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/sysoc.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/load_secure_fw.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/boot_ctl.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/smmu_config.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/sysdef_option.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gpio.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/pmu_driver.c")

target_link_libraries(
    ${SCP_MODULE_TARGET}
    PRIVATE module-ppu-v0-synquacer module-power-domain module-system-power
            module-f-i2c module-hsspi module-synquacer-memc module-timer
            module-nor)
