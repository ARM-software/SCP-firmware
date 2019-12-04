#
# Arm SCP/MCP Software
# Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Version
#
export VERSION_MAJOR := 2
export VERSION_MINOR := 6
export VERSION_PATCH := 0

#
# Directories
#
export TOP_DIR := $(shell pwd)
export ARCH_DIR := $(TOP_DIR)/arch
export DOC_DIR := $(TOP_DIR)/doc
export FWK_DIR := $(TOP_DIR)/framework
export TOOLS_DIR := $(TOP_DIR)/tools
export BS_DIR := $(TOOLS_DIR)/build_system
export PRODUCTS_DIR := $(TOP_DIR)/product
export MODULES_DIR := $(TOP_DIR)/module
export CMSIS_DIR := $(TOP_DIR)/cmsis/CMSIS/Core
export OS_DIR := $(TOP_DIR)/cmsis/CMSIS/RTOS2
export DBG_DIR := $(TOP_DIR)/debugger

BUILD_STRING := $(shell $(TOOLS_DIR)/build_string.py 2>/dev/null)
export VERSION_STRING = v$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)
export VERSION_DESCRIBE_STRING = $(VERSION_STRING)_$(BUILD_STRING)

#
# Tools
#
export RM := rm -rf
export MD := mkdir -p
export CP := cp -r
export DOC := doxygen

#
# Default options
#
DEFAULT_MODE := release
DEFAULT_BUILD_PATH := $(TOP_DIR)/build
DEFAULT_VERBOSE := n
DEFAULT_DEBUGGER := n

DEFAULT_LOG_LEVEL_debug := TRACE
DEFAULT_LOG_LEVEL_release := WARN

#
# Top-level configuration
#
.DEFAULT_GOAL = all
# MAKECMDGOALS isn't set if there's no explicit goal in the
# command line, so set the default.
MAKECMDGOALS ?= $(.DEFAULT_GOAL)

# Mode
export MODE ?= $(DEFAULT_MODE)
ifneq ($(filter-out debug release, $(MODE)),)
    $(error "Invalid MODE parameter. Aborting...")
endif

# Log level
export LOG_LEVEL ?= $(DEFAULT_LOG_LEVEL_${MODE})

# Build directory
BUILD_PATH ?= $(DEFAULT_BUILD_PATH)
ifeq ($(BUILD_PATH),)
    $(error "Invalid BUILD_PATH parameter. Aborting...")
endif
export BUILD_DIR = $(abspath $(BUILD_PATH))
export BUILD_DOC_DIR = $(BUILD_DIR)/doc

# Verbose mode: y/n
V ?= $(DEFAULT_VERBOSE)
export V
ifeq ($(V),n)
    MAKEFLAGS += --silent
    MAKEFLAGS += --no-print-directory
endif

# Include debugger library: y/n
DEBUGGER ?= $(DEFAULT_DEBUGGER)
ifeq ($(DEBUGGER),y)
    BUILD_HAS_DEBUGGER := yes
    export BUILD_HAS_DEBUGGER
endif

#
# Products
#
PRODUCTS := $(shell ls $(PRODUCTS_DIR) 2>/dev/null)

PRODUCT_INDEPENDENT_GOALS := clean doc help test lib-% module-%

ifneq ($(filter-out $(PRODUCT_INDEPENDENT_GOALS), $(MAKECMDGOALS)),)
    ifeq ($(PRODUCT),)
        $(error "You must define PRODUCT. Aborting...")
    endif

    export PRODUCT_DIR := $(PRODUCTS_DIR)/$(PRODUCT)

    ifeq ($(wildcard $(PRODUCT_DIR)/product.mk),)
        $(error "Missing product.mk in $(PRODUCT_DIR)")
    endif

    include $(PRODUCT_DIR)/product.mk

    ifeq ($(BS_FIRMWARE_LIST),)
        $(error "You must define BS_FIRMWARE_LIST in product.mk. Aborting...")
    endif

    FIRMWARE_TARGETS := $(addprefix firmware-, $(BS_FIRMWARE_LIST))

define msg_start
================================================================
Arm SCP/MCP Software build System
Platform    : $(BS_PRODUCT_NAME)
Mode        : $(MODE)
Firmware(s) : $(BS_FIRMWARE_LIST)
================================================================
endef

    $(info $(msg_start))
endif

#
# Rules
#

.PHONY: all
all: $(FIRMWARE_TARGETS)

module-%: $(MODULES_DIR)/%
	$(MAKE) -C $(MODULES_DIR)/$*/src

firmware-%: $(PRODUCT_DIR)/%
	$(MAKE) -f $(PRODUCT_DIR)/$*/firmware.mk FIRMWARE=$*

lib-%: $(TOP_DIR)/%
	$(MAKE) -C $*/src

.PHONY: test
test:
	$(MAKE) -C $(FWK_DIR)/test all

.PHONY: doc
doc:
	$(MAKE) -C $(DOC_DIR) doc

.PHONY: clean
clean:
	$(RM) $(BUILD_DIR)

.PHONY: help
help:
	@echo "Arm SCP/MCP Software build system"
	@echo ""
	@echo "Usage: make [PRODUCT=<name>] [OPTIONS] [TARGET]"
	@echo ""
	@echo "--------------------------------------------------------------------"
	@echo "| Available Targets                                                |"
	@echo "--------------------------------------------------------------------"
	@echo "    all             Build all firmware defined by PRODUCT=<product>"
	@echo "    clean           Remove all built products"
	@echo "    firmware-<name> Build a specific firmware from PRODUCT=<product>"
	@echo "    help            Show this documentation"
	@echo "    lib-<name>      Build a specific project library"
	@echo "    test            Build and run the framework test cases"
	@echo ""
	@echo "--------------------------------------------------------------------"
	@echo "| Product Selection                                                |"
	@echo "--------------------------------------------------------------------"
	@echo "    PRODUCT"
	@echo "        Available products: $(PRODUCTS)"
	@echo "        Select the target product to build. This is a required"
	@echo "        parameter."
	@echo ""
	@echo "--------------------------------------------------------------------"
	@echo "| Options                                                          |"
	@echo "--------------------------------------------------------------------"
	@echo "    BUILD_PATH"
	@echo "        Value: <Full, user provided path>"
	@echo "        Default: '$(DEFAULT_BUILD_PATH)'"
	@echo "        Set the base directory used during the build process."
	@echo ""
	@echo "    MODE"
	@echo "        Value: <debug | release>"
	@echo "        Default: '$(DEFAULT_MODE)'"
	@echo "        Choose between release and debug mode."
	@echo ""
	@echo "    O"
	@echo "        Value: <Compiler-specific optimization level>"
	@echo "        Default: <Compiler and MODE specific>"
	@echo "        Set the desired level of optimization the compiler will use."
	@echo ""
	@echo "    V"
	@echo "        Value: <y|n>"
	@echo "        Default: $(DEFAULT_VERBOSE)"
	@echo "        Enable or disable verbose mode for the build system."
	@echo ""
	@echo "    DEBUGGER"
	@echo "        Value: <y|n>"
	@echo "        Default: $(DEFAULT_DEBUGGER)"
	@echo "        Include the debugger library."
	@echo ""
	@echo "    LOG_LEVEL"
	@echo "        Value: <TRACE|INFO|WARN|ERROR|CRIT>"
	@echo "        Default: $(LOG_LEVEL)"
	@echo "        Filter log messages less important than this level."
	@echo ""
