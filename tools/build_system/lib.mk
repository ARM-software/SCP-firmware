#
# Arm SCP/MCP Software
# Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifndef BS_LIB_MK
BS_LIB_MK := 1

include $(BS_DIR)/cpu.mk
include $(BS_DIR)/defs.mk

$(info == Building $(BS_LIB_NAME) for $(BS_FIRMWARE_CPU))

SOURCES := $(BS_LIB_SOURCES)
INCLUDES += $(BS_LIB_INCLUDES)
DEFINES += $(BS_LIB_DEFINES)

ifeq ($(BUILD_FIRMWARE_DIR),)
    # We are not building underneath a particular firmware, so we use the target
    # CPU to identify where to build this library
    BUILD_FIRMWARE_DIR := $(BUILD_DIR)/$(BS_FIRMWARE_CPU)
endif

# Discard the product-specific path part of product-specific modules
LIB_BASE := $(subst product/$(PRODUCT)/,,$(shell pwd))

# Extract the part of the path that will be used as the build subdirectory
LIB_BASE := $(patsubst $(TOP_DIR)/%/src,%,$(LIB_BASE))

#
# Generate the name of the list that holds all of the disabled APIS for this
# library
#
DISABLED_APIS_LIST_NAME = \
    $(subst MODULE/,,$(call to_upper,$(LIB_BASE)))_DISABLED_APIS

# Define BUILD_DISABLE_API_<API> in this library for every disabled API
$(foreach api, \
          $($(DISABLED_APIS_LIST_NAME)), \
          $(eval DEFINES += BUILD_DISABLE_API_$(api)))


ifeq ($(BS_FIRMWARE_HAS_MULTITHREADING),yes)
    LIB_BASE := $(LIB_BASE)$(MULTHREADING_SUFFIX)
endif

ifeq ($(BS_FIRMWARE_HAS_NOTIFICATION),yes)
    LIB_BASE := $(LIB_BASE)$(NOTIFICATION_SUFFIX)
endif

LIB_DIR = $(BUILD_FIRMWARE_DIR)/$(LIB_BASE)/$(MODE)/lib
OBJ_DIR = $(BUILD_FIRMWARE_DIR)/$(LIB_BASE)/$(MODE)/obj

LIB = $(LIB_DIR)/lib.a

goal: $(LIB)

INCLUDES += $(shell pwd)
INCLUDES += $(TOP_DIR)/$(LIB_BASE)/include

INCLUDES += $(ARCH_DIR)/$(BS_ARCH_VENDOR)/include
INCLUDES += $(ARCH_DIR)/$(BS_ARCH_VENDOR)/$(BS_ARCH_ARCH)/include

include $(BS_DIR)/rules.mk

endif
