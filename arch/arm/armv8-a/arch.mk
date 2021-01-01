#
# Arm SCP/MCP Software
# Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_cache_helpers.S
BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_crt0.S
BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_exceptions.c
BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_gic.c
BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_libc.c
BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_main.c
BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_misc_helpers.S
BS_LIB_SOURCES_$(BS_ARCH_ARCH) += arch_mm.c

BS_LIB_SOURCES_$(BS_ARCH_ARCH) := $(addprefix $(ARCH_DIR)/$(BS_ARCH_VENDOR)/$(BS_ARCH_ARCH)/src/,$(BS_LIB_SOURCES_$(BS_ARCH_ARCH)))
