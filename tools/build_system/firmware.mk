#
# Arm SCP/MCP Software
# Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include $(BS_DIR)/defs.mk

$(info == Building firmware $(FIRMWARE))

#
# Validate that all mandatory definitions are defined
#
ifeq ($(BS_FIRMWARE_CPU),)
    $(error "You must define BS_FIRMWARE_CPU. Aborting...")
endif

ifeq ($(BS_FIRMWARE_HAS_MULTITHREADING),)
    $(error "You must define BS_FIRMWARE_HAS_MULTITHREADING. \
             Valid options are: 'yes' and 'no'. \
             Aborting...")
endif

ifeq ($(BS_FIRMWARE_HAS_NOTIFICATION),)
    $(error "You must define BS_FIRMWARE_HAS_NOTIFICATION. \
             Valid options are: 'yes' and 'no'. \
             Aborting...")
endif

ifneq ($(filter-out yes no,$(BS_FIRMWARE_HAS_MULTITHREADING)),)
    $(error "Invalid parameter for BS_FIRMWARE_HAS_MULTITHREADING. \
             Valid options are: 'yes' and 'no'. \
             Aborting...")
endif

ifneq ($(filter-out yes no,$(BS_FIRMWARE_HAS_NOTIFICATION)),)
    $(error "Invalid parameter for BS_FIRMWARE_HAS_NOTIFICATION. \
             Valid options are: 'yes' and 'no'. \
             Aborting...")
endif

export BS_FIRMWARE_CPU
export BS_FIRMWARE_HAS_MULTITHREADING
export BS_FIRMWARE_HAS_NOTIFICATION

#
# Generate FIRMWARE_MODULES_LIST and list of excluded APIs for each module in
# the following format:
#
# <MODULE_NAME>_DISABLED_APIS = <API_1> <API_2> ...
#
# NOTE: <MODULE_NAME> and <API_n> are all in uppercase
#
SPLIT_INPUT = $(subst $(comma), ,$(1))
GET_MODULE_NAME = $(word 1,$(SPLIT_INPUT))

define process_module_entry
    export $(call to_upper,$(GET_MODULE_NAME))_DISABLED_APIS = \
    $(call to_upper,$(filter-out $(GET_MODULE_NAME),$(SPLIT_INPUT)))

    FIRMWARE_MODULES_LIST += $(GET_MODULE_NAME)
    DEFINES += BUILD_HAS_MOD_$(call to_upper,$(GET_MODULE_NAME))=1
endef

$(foreach entry, \
          $(BS_FIRMWARE_MODULES), \
          $(eval $(call process_module_entry, $(entry))))

include $(BS_DIR)/cpu.mk

BUILD_PRODUCT_DIR := $(BUILD_DIR)/product/$(PRODUCT)
BUILD_FIRMWARE_DIR := $(BUILD_PRODUCT_DIR)/$(FIRMWARE)
BIN_DIR := $(BUILD_FIRMWARE_DIR)/$(MODE)/bin
OBJ_DIR := $(BUILD_FIRMWARE_DIR)/$(MODE)/obj
PRODUCT_MODULES_DIR := $(PRODUCT_DIR)/module
FIRMWARE_DIR := $(PRODUCT_DIR)/$(FIRMWARE)

TARGET := $(BIN_DIR)/firmware
TARGET_BIN := $(TARGET).bin
TARGET_ELF := $(TARGET).elf

vpath %.c $(FIRMWARE_DIR)
vpath %.S $(FIRMWARE_DIR)
vpath %.c $(PRODUCT_DIR)/src
vpath %.S $(PRODUCT_DIR)/src

goal: $(TARGET_BIN)

ifneq ($(BS_ARCH_CPU),host)
    SCATTER_SRC = $(ARCH_DIR)/src/$(BS_ARCH_ARCH)/ld.S
    SCATTER_PP = $(OBJ_DIR)/ld_preproc.s
endif

#
# Sources
#
SOURCES = $(BS_FIRMWARE_SOURCES)

#
# Modules
#
ALL_STANDARD_MODULES := $(shell ls $(MODULES_DIR) 2>/dev/null)
ALL_PRODUCT_MODULES := $(shell ls $(PRODUCT_MODULES_DIR) 2>/dev/null)

# Check for conflicts between module names
CONFLICTING_MODULES := $(filter $(ALL_PRODUCT_MODULES), $(ALL_STANDARD_MODULES))
ifneq ($(CONFLICTING_MODULES),)
    $(error "The following module(s) in '$(PRODUCT)' conflict with modules of \
             the same name in $(MODULES_DIR): $(CONFLICTING_MODULES). \
             Aborting...")
endif

# Check for missing or invalid modules
MISSING_MODULES := $(filter-out $(ALL_STANDARD_MODULES) $(ALL_PRODUCT_MODULES),\
                   $(FIRMWARE_MODULES_LIST) $(BS_FIRMWARE_MODULE_HEADERS_ONLY))
ifneq ($(MISSING_MODULES),)
    $(error "Missing or invalid module(s): $(MISSING_MODULES). Aborting...")
endif

# Modules selected to be built into the firmware
BUILD_STANDARD_MODULES := $(filter $(ALL_STANDARD_MODULES), \
                                   $(FIRMWARE_MODULES_LIST))
BUILD_PRODUCT_MODULES := $(filter $(ALL_PRODUCT_MODULES), \
                                  $(FIRMWARE_MODULES_LIST))

# Module selected to have their headers made available for inclusion by other
# modules and their configuration files. These modules are not built into the
# firmware.
HEADER_STANDARD_MODULES := $(filter $(BS_FIRMWARE_MODULE_HEADERS_ONLY), \
                                    $(ALL_STANDARD_MODULES))
HEADER_PRODUCT_MODULES := $(filter $(BS_FIRMWARE_MODULE_HEADERS_ONLY), \
                                   $(ALL_PRODUCT_MODULES))

ifeq ($(BS_FIRMWARE_HAS_MULTITHREADING),yes)
    BUILD_SUFFIX := $(MULTHREADING_SUFFIX)
    BUILD_HAS_MULTITHREADING := yes

    ifneq ($(BS_ARCH_ARCH),host)
      LIBS_y += $(OS_DIR)/RTX/Library/GCC/libRTX_CM3.a
    endif

    INCLUDES += $(OS_DIR)/RTX/Source
    INCLUDES += $(OS_DIR)/RTX/Include
    INCLUDES += $(OS_DIR)/../Core/Include
else
    BUILD_HAS_MULTITHREADING := no
endif
export BUILD_HAS_MULTITHREADING

ifeq ($(BS_FIRMWARE_HAS_NOTIFICATION),yes)
    BUILD_SUFFIX := $(BUILD_SUFFIX)$(NOTIFICATION_SUFFIX)
    BUILD_HAS_NOTIFICATION := yes
else
    BUILD_HAS_NOTIFICATION := no
endif
export BUILD_HAS_NOTIFICATION


# Add directories to the list of targets to build
LIB_TARGETS_y += $(patsubst %,$(MODULES_DIR)/%/src, \
                            $(BUILD_STANDARD_MODULES))
LIB_TARGETS_y += $(patsubst %,$(PRODUCT_MODULES_DIR)/%/src, \
                            $(BUILD_PRODUCT_MODULES))

# Add lib path to the list of modules to link
MODULE_LIBS_y += $(patsubst %, \
    $(BUILD_FIRMWARE_DIR)/module/%$(BUILD_SUFFIX)/$(MODE)/lib/lib.a, \
    $(BUILD_STANDARD_MODULES) $(BUILD_PRODUCT_MODULES))

# Create a list of include directories from the selected modules
MODULE_INCLUDES += $(patsubst %,$(MODULES_DIR)/%/include, \
                              $(BUILD_STANDARD_MODULES))
MODULE_INCLUDES += $(patsubst %,$(PRODUCT_MODULES_DIR)/%/include, \
                              $(BUILD_PRODUCT_MODULES))
MODULE_INCLUDES += $(patsubst %,$(MODULES_DIR)/%/include, \
                              $(HEADER_STANDARD_MODULES))
MODULE_INCLUDES += $(patsubst %,$(PRODUCT_MODULES_DIR)/%/include, \
                              $(HEADER_PRODUCT_MODULES))

# Default product include directories
PRODUCT_INCLUDES += $(PRODUCT_DIR)
PRODUCT_INCLUDES += $(PRODUCT_DIR)/include

# Add the firmware directory to the main INCLUDES list
INCLUDES += $(FIRMWARE_DIR)

# Add module and product includes to the main INCLUDES list
export INCLUDES += $(MODULE_INCLUDES) $(PRODUCT_INCLUDES)

#
# Standard libraries
#
LIB_TARGETS_y += $(ARCH_DIR)/src
LIB_TARGETS_y += $(FWK_DIR)/src

LIBS_y += $(call lib_path,arch$(BUILD_SUFFIX))
LIBS_y += $(call lib_path,framework$(BUILD_SUFFIX))

SOURCES += $(BUILD_FIRMWARE_DIR)/fwk_module_list.c
$(BUILD_FIRMWARE_DIR)/fwk_module_list.c: gen_module
EXTRA_DEP := gen_module
export EXTRA_DEP
export BUILD_FIRMWARE_DIR
export FIRMWARE_MODULES_LIST
export DEFINES

include $(BS_DIR)/rules.mk

LDFLAGS_GCC += -Wl,-Map=$(TARGET).map

ifneq ($(BS_ARCH_CPU),host)
    LDFLAGS_GCC += -Wl,-n
endif

$(TARGET_ELF): $(LIB_TARGETS_y) $(SCATTER_PP) $(OBJECTS) | $(targetdir)
	$(call show-action,LD,$@)
	$(LD) $(LDFLAGS) \
	     -Wl,--start-group \
	        $(BUILTIN_LIBS) \
	        $(MODULE_LIBS_y) \
	        $(LIBS_y) \
	        $(OBJECTS) \
	    -Wl,--end-group \
	    -o $@
	$(SIZE) $@

$(SCATTER_PP): $(SCATTER_SRC) | $(targetdir)
	$(call show-action,GEN,$@)
	$(CC) $(CFLAGS) -E -P -C $< -o $@

$(TARGET_BIN): $(TARGET_ELF)
	$(call show-action,BIN,$@)
	$(OBJCOPY) -O binary $< $@
