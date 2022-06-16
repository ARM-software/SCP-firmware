#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Directories
#
export TOP_DIR := $(shell pwd)
export ARCH_DIR := $(TOP_DIR)/arch
export FWK_DIR := $(TOP_DIR)/framework
export TOOLS_DIR := $(TOP_DIR)/tools
export BS_DIR := $(TOOLS_DIR)/build_system
export PRODUCTS_DIR := $(TOP_DIR)/product
export MODULES_DIR := $(TOP_DIR)/module
export DOC_DIR := $(TOP_DIR)/doc

#
# Tools
#
export RM := rm -rf
export MD := mkdir -p
export CP := cp -r
export DOC := doxygen
export CMAKE := cmake
export CTEST := ctest
export CD := cd

#
# Documentation paths
#
export BUILD_STRING := $(shell $(TOOLS_DIR)/build_string.py 2>/dev/null)

export BUILD_DOC_DIR = $(BUILD_DIR)/doc
export MODULE_INCLUDES := $(shell ls -d $(MODULES_DIR)/*/include/ 2>/dev/null)
export MODULE_DOCS := $(shell ls -d $(MODULES_DIR)/*/doc/ 2>/dev/null)
export MODULE_DOCS += $(shell ls -d $(PRODUCTS_DIR)/*/module/*/doc/ 2>/dev/null)

#
# Default options
#
DEFAULT_MODE := release
DEFAULT_BUILD_PATH := $(TOP_DIR)/build
DEFAULT_VERBOSE := n
DEFAULT_DEBUGGER := n
DEFAULT_TOOLCHAIN := GNU
DEFAULT_BUILD_SYSTEM := Ninja
export CMSIS_DIR := $(TOP_DIR)/contrib/cmsis/git/CMSIS/Core
DEFAULT_LOG_LEVEL_debug := INFO
DEFAULT_LOG_LEVEL_release := WARN

DEFAULT_CMAKE_TOOL_LOG_LEVEL := NOTICE

#
# Top-level configuration
#
.DEFAULT_GOAL = all
# MAKECMDGOALS isn't set if there's no explicit goal in the
# command line, so set the default.
MAKECMDGOALS ?= $(.DEFAULT_GOAL)

# Mode
MODE ?= $(DEFAULT_MODE)
ifneq ($(filter-out debug release, $(MODE)),)
    $(error "Invalid MODE parameter. Aborting...")
endif

# Build System
BUILD_SYSTEM ?= $(DEFAULT_BUILD_SYSTEM)
ifneq ($(filter-out Make Ninja, $(BUILD_SYSTEM)),)
    $(error "Invalid BUILD_SYSTEM parameter. Aborting...")
endif

# Toolchain
TOOLCHAIN ?= $(DEFAULT_TOOLCHAIN)
ifneq ($(filter-out GNU ArmClang Clang, $(TOOLCHAIN)),)
    $(error "Invalid TOOLCHAIN parameter. Aborting...")
endif

# Log level
LOG_LEVEL ?= $(DEFAULT_LOG_LEVEL_${MODE})
CMAKE_SCP_LOG_LEVEL_OPTION=-DSCP_LOG_LEVEL=$(LOG_LEVEL)

# Build directory
BUILD_PATH ?= $(DEFAULT_BUILD_PATH)
ifeq ($(BUILD_PATH),)
    $(error "Invalid BUILD_PATH parameter. Aborting...")
endif
export BUILD_DIR = $(abspath $(BUILD_PATH))

# Verbose mode: y/n
export V ?= $(DEFAULT_VERBOSE)
ifneq ($(V),n)
    CMAKE_TOOL_LOG_LEVEL := --log-level=VERBOSE
    CMAKE_BUILD_VERBOSE_OPTION := -v
else
    CMAKE_TOOL_LOG_LEVEL := --log-level=$(DEFAULT_CMAKE_TOOL_LOG_LEVEL)
    MAKEFLAGS += --silent
    MAKEFLAGS += --no-print-directory
endif

# Include debugger library: y/n
DEBUGGER ?= $(DEFAULT_DEBUGGER)
ifeq ($(DEBUGGER),y)
    CMAKE_DEBUGGER_OPTION := -DSCP_ENABLE_DEBUGGER=ON
endif

#
# Products
#
PRODUCTS := $(shell ls $(PRODUCTS_DIR) 2>/dev/null)

PRODUCT_INDEPENDENT_GOALS := clean help test doc

ifneq ($(filter-out $(PRODUCT_INDEPENDENT_GOALS), $(MAKECMDGOALS)),)
    ifeq ($(PRODUCT),)
        $(error "You must define PRODUCT. Aborting...")
    endif

    PRODUCT_DIR := $(PRODUCTS_DIR)/$(PRODUCT)

    ifeq ($(wildcard $(PRODUCT_DIR)/product.mk),)
        $(error "Missing product.mk in $(PRODUCT_DIR)")
    endif

    include $(PRODUCT_DIR)/product.mk

    ifeq ($(BS_FIRMWARE_LIST),)
        $(error "You must define BS_FIRMWARE_LIST in product.mk. Aborting...")
    endif

    ifeq ($(BUILD_TARGET),)
        FIRMWARE_TARGETS := $(addprefix firmware-, $(BS_FIRMWARE_LIST))
    else
        ifneq ($(findstring $(BUILD_TARGET),$(BS_FIRMWARE_LIST)),)
            FIRMWARE_TARGETS := $(addprefix firmware-, $(BUILD_TARGET))
        else
            $(error "Specific BUILD_TARGET $(BUILD_TARGET) not in valid target list: $(BS_FIRMWARE_LIST)")
        endif
    endif


    ifndef PLATFORM_VARIANT
	    PRODUCT_BUILD_PATH := $(BUILD_PATH)/$(BS_PRODUCT_NAME)/$(TOOLCHAIN)/$(MODE)
    else
	    PRODUCT_BUILD_PATH := $(BUILD_PATH)/$(BS_PRODUCT_NAME)/platform_variant_$(PLATFORM_VARIANT)/$(TOOLCHAIN)/$(MODE)
    endif

	GENERATE_PREFIXES := $(addprefix $(PRODUCT_BUILD_PATH)/, $(FIRMWARE_TARGETS))
	GENERATE_TARGETS := $(addsuffix /CMakeCache.txt, $(GENERATE_PREFIXES))

    ifneq ($(CPP_HALT),)
          CPP_HALT_ARGS := -DSCP_BUILD_CPP_ONLY=1
    endif

define msg_start
================================================================
Arm SCP/MCP Software build System
Platform    : $(BS_PRODUCT_NAME)
Mode        : $(MODE)
Firmware(s) : $(BS_FIRMWARE_LIST)
Targets(s)  : $(FIRMWARE_TARGETS)
================================================================
endef

    $(info $(msg_start))

endif

ifeq ($(MODE),debug)
	CMAKE_BUILD_TYPE=-DCMAKE_BUILD_TYPE=Debug
endif

CMAKE_COMMAND_OPTION := $(CMAKE_BUILD_TYPE)
CMAKE_COMMAND_OPTION += -DSCP_TOOLCHAIN="$(TOOLCHAIN)"
CMAKE_COMMAND_OPTION += -DSCP_LLVM_SYSROOT_CC="$(LLVM_SYSROOT_CC)"

ifdef PLATFORM_VARIANT
    CMAKE_COMMAND_OPTION += -DSCP_PLATFORM_VARIANT="$(PLATFORM_VARIANT)"
endif

ifdef CMAKE_BUILD_VERBOSE_OPTION
    CMAKE_COMMAND_OPTION = -DCOMMAND_OUTPUT_VERBOSE=1
endif

ifeq ($(BUILD_SYSTEM), Ninja)
    CMAKE_COMMAND_OPTION += -G $(BUILD_SYSTEM)
endif

CMAKE_COMMAND_OPTION += $(CMAKE_TOOL_LOG_LEVEL)
CMAKE_COMMAND_OPTION += $(CMAKE_SCP_LOG_LEVEL_OPTION)
CMAKE_COMMAND_OPTION += $(CMAKE_DEBUGGER_OPTION)
CMAKE_COMMAND_OPTION += $(CPP_HALT_ARGS)

#
# Rules
#
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
	@echo "    generate        Generate all CMake build trees defined by "
	@echo "                    PRODUCT=<product>."
	@echo "    clean           Remove all built products"
	@echo "    help            Show this documentation"
	@echo "    doc             Generate the documentation of this project with Doxygen"
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
	@echo "    BUILD_SYSTEM"
	@echo "        Value: <Make|Ninja>"
	@echo "        Default: $(DEFAULT_BUILD_SYSTEM)"
	@echo "        Specify CMake to generate GNU Make or Ninja build system."
	@echo ""
	@echo "    BUILD_TARGET"
	@echo "        Value: <Single firmware target variant valid for PRODUCT>"
	@echo "        Default: "
	@echo "        Specify a named firmware target for the build - otherwise will"
	@echo "        build all valid firmware targets for the selected PRODUCT"
	@echo ""
	@echo "    TOOLCHAIN"
	@echo "        Value: <GNU|ArmClang|Clang>"
	@echo "        Default: $(DEFAULT_TOOLCHAIN)"
	@echo "        Specify Toolchain to build the firmware."
	@echo ""
	@echo "    LLVM_SYSROOT_CC"
	@echo "        Value: <LLVM sysroot compiler path>"
	@echo "        Specify LLVM sysroot compiler path to build the firmware."
	@echo ""
	@echo "    PLATFORM_VARIANT"
	@echo "        Value: <Platform variant>"
	@echo "        Specify Platform variant if it is required."
	@echo ""
	@echo "    EXTRA_CONFIG_ARGS"
	@echo "        Value: <cmake configuration parameters>"
	@echo "        Default: "
	@echo "        Pass extra arguments directly to cmake configuration stage."
	@echo "        Multiple extra args can be added with += or by passing the arguments as a string"
	@echo ""
	@echo "    EXTRA_BUILD_ARGS"
	@echo "        Value: <cmake build parameters>"
	@echo "        Default: "
	@echo "        Pass extra arguments directly to cmake build stage."
	@echo "        Multiple extra args can be added with += or by passing the arguments as a string"
	@echo ""
	@echo "    CPP_HALT"
	@echo "        Value: 1 "
	@echo "        Default: "
	@echo "        Halt after the C Pre-Processor stage - use to debug macro expansion"
	@echo ""

.SECONDEXPANSION:

.PHONY: all
all: command_msg $(FIRMWARE_TARGETS)

define msg_generate
================================================================
Generate only: $(GENERATE_TARGETS)
================================================================
endef

.PHONY: generate
generate: generate_msg $(GENERATE_TARGETS)

generate_msg:
	$(info $(msg_generate))


define msg_cmd
CMake Commnd options: $(CMAKE_COMMAND_OPTION)
endef

command_msg:
    $(info $(msg_cmd))

firmware-%: $(PRODUCT_BUILD_PATH)/$$@/CMakeCache.txt
	$(CMAKE) --build $(<D)/ $(CMAKE_BUILD_VERBOSE_OPTION) $(EXTRA_BUILD_ARGS)

.PRECIOUS: $(PRODUCT_BUILD_PATH)/firmware-%/CMakeCache.txt

$(PRODUCT_BUILD_PATH)/firmware-%/CMakeCache.txt:  $(PRODUCT_DIR)/%/Firmware.cmake
	$(RM) $(@D)
	$(CMAKE) -B $(@D) -DSCP_FIRMWARE_SOURCE_DIR:PATH=$(PRODUCT_DIR)/$* $(CMAKE_COMMAND_OPTION) $(EXTRA_CONFIG_ARGS)

.PHONY: clean
clean:
	$(RM) $(BUILD_DIR)


.PHONY: doc
doc:
	mkdir -p $(BUILD_DOC_DIR)
	$(DOC) $(DOC_DIR)/Doxyfile

.PHONY: test
test:
	$(CMAKE) -B ${BUILD_PATH}/framework/test $(FWK_DIR)/test -G Ninja
	$(CMAKE) --build ${BUILD_PATH}/framework/test
	# --test-dir option of ctest is not available before ctest 3.20
	# so use workaround to change the test dir and run the tests from there
	${CD} ${BUILD_PATH}/framework/test && ${CTEST} -V
