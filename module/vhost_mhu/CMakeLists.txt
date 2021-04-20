#
# Arm SCP/MCP Software
# Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")


if(SCP_ARCHITECTURE_TARGET STREQUAL "arch-none")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0 gio-unix-2.0)
    target_link_libraries(${SCP_MODULE_TARGET} PRIVATE PkgConfig::deps)

    target_include_directories(${SCP_MODULE_TARGET}
                           PRIVATE "/home/vingu/Tools/Dev/qemu/")

    add_library( lib-vhost-user STATIC IMPORTED )
    set_target_properties( lib-vhost-user PROPERTIES IMPORTED_LOCATION /usr/local/lib/libvhost-user.a)
    target_link_libraries(${SCP_MODULE_TARGET} PRIVATE lib-vhost-user)

    add_library( lib-vhost-user-glib STATIC IMPORTED )
    set_target_properties( lib-vhost-user-glib PROPERTIES IMPORTED_LOCATION /usr/local/lib/libvhost-user-glib.a)
    target_link_libraries(${SCP_MODULE_TARGET} PRIVATE lib-vhost-user-glib)
endif()

target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-msg-smt)

target_sources(${SCP_MODULE_TARGET}
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_vhost_mhu.c")
