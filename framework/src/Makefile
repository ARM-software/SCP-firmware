#
# Arm SCP/MCP Software
# Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_LIB_NAME := framework

BS_LIB_SOURCES += fwk_arch.c
BS_LIB_SOURCES += fwk_assert.c
BS_LIB_SOURCES += fwk_dlist.c
BS_LIB_SOURCES += fwk_id.c
BS_LIB_SOURCES += fwk_interrupt.c
BS_LIB_SOURCES += fwk_log.c
BS_LIB_SOURCES += fwk_mm.c
BS_LIB_SOURCES += fwk_module.c
BS_LIB_SOURCES += fwk_ring.c
BS_LIB_SOURCES += fwk_slist.c
BS_LIB_SOURCES += fwk_thread_delayed_resp.c
ifeq ($(BUILD_HAS_MULTITHREADING),yes)
    BS_LIB_SOURCES += fwk_multi_thread.c
else
    BS_LIB_SOURCES += fwk_thread.c
endif
ifeq ($(BUILD_HAS_NOTIFICATION),yes)
    BS_LIB_SOURCES += fwk_notification.c
endif
BS_LIB_SOURCES += fwk_status.c

BS_LIB_INCLUDES += $(ARCH_DIR)/include
BS_LIB_INCLUDES += $(FWK_DIR)/include

include $(BS_DIR)/lib.mk
