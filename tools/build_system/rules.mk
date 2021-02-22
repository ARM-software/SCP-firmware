#
# Arm SCP/MCP Software
# Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifndef BS_RULES_MK
BS_RULES_MK := 1

include $(BS_DIR)/defs.mk
include $(BS_DIR)/cpu.mk

ifeq ($(BUILD_HAS_MULTITHREADING),yes)
    # Add the OS directory to the main INCLUDES list
    ifeq ($(findstring $(BS_FIRMWARE_CPU),$(ARMV8A_CPUS)),)
        INCLUDES += $(OS_DIR)/Include
    endif
    DEFINES += BUILD_HAS_MULTITHREADING
endif

ifeq ($(BUILD_HAS_NOTIFICATION),yes)
    DEFINES += BUILD_HAS_NOTIFICATION
endif

ifeq ($(BUILD_HAS_SCMI_NOTIFICATIONS),yes)
    DEFINES += BUILD_HAS_SCMI_NOTIFICATIONS
endif

ifeq ($(BUILD_HAS_FAST_CHANNELS),yes)
    DEFINES += BUILD_HAS_FAST_CHANNELS
endif

ifeq ($(BUILD_HAS_SCMI_RESET),yes)
    DEFINES += BUILD_HAS_SCMI_RESET
endif

ifeq ($(BUILD_HAS_STATISTICS),yes)
    DEFINES += BUILD_HAS_STATISTICS
endif

ifeq ($(BUILD_HAS_RESOURCE_PERMISSIONS),yes)
    DEFINES += BUILD_HAS_RESOURCE_PERMISSIONS
endif

ifeq ($(BUILD_HAS_SCMI_SENSOR_EVENTS),yes)
    ifeq ($(BUILD_HAS_SCMI_NOTIFICATIONS),yes)
            DEFINES += BUILD_HAS_SCMI_SENSOR_EVENTS
    endif
endif

export AS := $(CC)
export LD := $(CC)

include $(BS_DIR)/toolchain.mk

ifeq ($(BS_LINKER),ARM)
    export AR := $(shell $(CC) --print-prog-name armar)
    export OBJCOPY := $(shell $(CC) --print-prog-name fromelf)
    export SIZE := $(shell $(CC) --print-prog-name size)
else ifeq ($(BS_TOOLCHAIN),LLVM)
    export LLVM_BIN_DIR := $(shell $(CC) --print-resource-dir)/../../../bin
    export AR := $(LLVM_BIN_DIR)/llvm-ar
    export OBJCOPY := $(LLVM_BIN_DIR)/llvm-objcopy
    export SIZE := $(LLVM_BIN_DIR)/llvm-size

    # Clang currently does not use LLD for all the ARM targets,
    # therefore we are enforcing the use of it manually.
    export LD := $(LLVM_BIN_DIR)/ld.lld
else
    export AR := $(shell $(CC) --print-prog-name ar)
    export OBJCOPY := $(shell $(CC) --print-prog-name objcopy)
    export SIZE := $(shell $(CC) --print-prog-name size)
endif

#
# GCC-specific optimization levels for debug and release modes
#

DEFAULT_OPT_GCC_DEBUG := g
DEFAULT_OPT_GCC_RELEASE := 2

#
# Compiler options used when building for the host
#
ifeq ($(BS_ARCH_ARCH),host)
    CFLAGS  += -mtune=native
    ASFLAGS_GCC += -mtune=native
    LDFLAGS_GCC += -mtune=native

    ifeq ($(BUILD_HAS_MULTITHREADING),yes)
        LDFLAGS_GCC += -pthread
    endif

    DEFINES += BUILD_HOST

#
# Compiler options used when cross compiling
#
else
    LDFLAGS_ARM += -Wl,--scatter=$(SCATTER_PP)
    LDFLAGS_GCC += -Wl,--script=$(SCATTER_PP)

    ifeq ($(BS_TOOLCHAIN),LLVM)
        # big.LITTLE tuning is not supported by LLVM
        ifneq ($(findstring .,$(BS_ARCH_CPU)),)
            CFLAGS  += -march=$(BS_ARCH_ARCH)
            ASFLAGS_GCC += -march=$(BS_ARCH_ARCH)
        else
            CFLAGS  += -mcpu=$(BS_ARCH_CPU)
            ASFLAGS_GCC += -mcpu=$(BS_ARCH_CPU)
        endif
    else
        CFLAGS  += -mcpu=$(BS_ARCH_CPU)
        ASFLAGS_GCC += -mcpu=$(BS_ARCH_CPU)
        LDFLAGS_GCC += -mcpu=$(BS_ARCH_CPU)
        LDFLAGS_ARM += -mcpu=$(BS_ARCH_CPU)
    endif

    # Optional architectural mode parameter
    ifneq ($(BS_ARCH_MODE),)
        CFLAGS  += -m$(BS_ARCH_MODE)
        ASFLAGS_GCC += -m$(BS_ARCH_MODE)
        LDFLAGS_GNU += -m$(BS_ARCH_MODE)
        LDFLAGS_ARM += -m$(BS_ARCH_MODE)
    endif
endif

#
# Warning flags
#

# The following flags are enforced to minimise unwitting uses of undefined
# behaviour in the code base, which can open security holes. Each flag applies a
# set of warnings, and any warnings that do occur are upgraded to errors to
# prevent the firmware from building.
CFLAGS += -Werror
CFLAGS += -Wall
CFLAGS += -Wextra

CFLAGS += -Wno-error=deprecated-declarations
CFLAGS += -Wno-unused-parameter

# GCC is not currently consistent in how it applies this warning, but this flag
# should be removed should we move to a version that can build the firmware
# without it.
CFLAGS += -Wno-missing-field-initializers

# Place functions and data into their own sections. This allows the linker to
# strip out functions with no references.
CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS_GCC += -Wl,--gc-sections
LDFLAGS_ARM += -Wl,--remove

# Arm Compiler 6 uses dollar symbols in its linker-defined symbol names
CFLAGS_CLANG += -Wno-dollar-in-identifier-extension

CFLAGS += -g
CFLAGS += -std=gnu11
CFLAGS_CLANG += -fshort-enums # Required by RTX

CFLAGS += -fno-exceptions

CPPFLAGS += -x c -E -P

DEP_CFLAGS_GCC = -MD -MP

ASFLAGS_GCC += -x
ASFLAGS_GCC += assembler-with-cpp
ASFLAGS_GCC += -g

ARFLAGS_GCC = -rc

LDFLAGS_GCC += -Wl,--cref

LDFLAGS_GCC += -Wl,--undefined=arch_exceptions
LDFLAGS_ARM += -Wl,--undefined=arch_exceptions

ifeq ($(BS_TOOLCHAIN),LLVM)
    ifneq ($(BS_ARCH_ARCH),armv8-a)
        ifeq ($(SYSROOT_CC),)
            $(error "You must define SYSROOT_CC. Aborting...")
        endif

        SYSROOT := $(shell $(SYSROOT_CC) -print-sysroot)
        SYSROOT_CMD := $(SYSROOT_CC) -mcpu=$(BS_ARCH_CPU)

        LDFLAGS_GCC += -L$(SYSROOT)/lib/$(shell $(SYSROOT_CMD) -print-multi-directory)

        BUILTIN_LIBS_GCC := \
            $(shell $(SYSROOT_CMD) --print-file-name=crti.o) \
            $(shell $(SYSROOT_CMD) --print-file-name=crtbegin.o) \
            $(shell $(SYSROOT_CMD) --print-file-name=crt0.o) \
            $(shell $(SYSROOT_CMD) --print-file-name=crtend.o) \
            $(shell $(SYSROOT_CMD) --print-file-name=crtn.o)
    else
        ifeq ($(SYSROOT),)
            $(error "You must define SYSROOT. Aborting...")
        endif

        LDFLAGS_GCC += -L$(SYSROOT)/lib
    endif

    CFLAGS += --sysroot=$(SYSROOT) -I$(SYSROOT)/include
endif

ifneq ($(BS_ARCH_ARCH),armv8-a)
    ifeq ($(BS_TOOLCHAIN),LLVM)
        ifeq ($(BS_FIRMWARE_USE_NEWLIB_NANO_SPECS),yes)
            LIBS_GROUP_END += -lc_nano
        else
            LIBS_GROUP_END += -lc -lnosys
        endif

        LDFLAGS_GCC += -L$(shell $(CC) -print-resource-dir)/lib/baremetal

        BUILTIN_LIBS_GCC += -nostdlib -lclang_rt.builtins-$(CLANG_BUILTINS_ARCH)
    else
        BUILTIN_LIBS_GCC := -lc -lgcc
    endif
else
    BUILTIN_LIBS_GCC := -nostdlib
endif

ifeq ($(MODE),release)
    O ?= $(DEFAULT_OPT_GCC_RELEASE)
    LOG_LEVEL ?= $(DEFAULT_LOG_LEVEL_RELEASE)

    # Disable assertions in release mode
    DEFINES += NDEBUG
else
    O ?= $(DEFAULT_OPT_GCC_DEBUG)
    LOG_LEVEL ?= $(DEFAULT_LOG_LEVEL_DEBUG)

    DEFINES += BUILD_MODE_DEBUG
endif

DEFINES += FWK_LOG_LEVEL=FWK_LOG_LEVEL_$(LOG_LEVEL)

ifeq ($(BUILD_HAS_DEBUGGER),yes)
    DEFINES += BUILD_HAS_DEBUGGER
endif

#
# Always include the architecture librarie
#

INCLUDES += $(ARCH_DIR)/include
INCLUDES += $(ARCH_DIR)/$(BS_ARCH_VENDOR)/include
INCLUDES += $(ARCH_DIR)/$(BS_ARCH_VENDOR)/$(BS_ARCH_ARCH)/include

ifeq ($(BS_ARCH_ARCH),armv8-a)
    INCLUDES += $(ARCH_DIR)/$(BS_ARCH_VENDOR)/$(BS_ARCH_ARCH)/include/common
    INCLUDES += $(ARCH_DIR)/$(BS_ARCH_VENDOR)/$(BS_ARCH_ARCH)/include/lib
    INCLUDES += $(ARCH_DIR)/$(BS_ARCH_VENDOR)/$(BS_ARCH_ARCH)/include/lib/libc
    INCLUDES += $(ARCH_DIR)/$(BS_ARCH_VENDOR)/$(BS_ARCH_ARCH)/include/lib/libc/aarch64
endif

#
# Always include the framework library
#
INCLUDES += $(FWK_DIR)/include

#
# CMSIS library
#
ifneq ($(BS_ARCH_ARCH),armv8-a)
    INCLUDES += $(CMSIS_DIR)/Include
endif

#
# Toolchain-independent flags
#
CFLAGS += -O$(O)
CFLAGS += $(addprefix -I,$(INCLUDES)) $(addprefix -D,$(DEFINES))
ASFLAGS += $(addprefix -I,$(INCLUDES)) $(addprefix -D,$(DEFINES))

#
# Assign toolchain-specific flags
#
ASFLAGS += $(ASFLAGS_GCC)
ARFLAGS = $(ARFLAGS_GCC)

ifeq ($(BS_TOOLCHAIN),LLVM)
    # Remove all pass to linker flags as we are using the linker directly
    TO_REMOVE := -Wl,
    LDFLAGS += $(subst $(TO_REMOVE),,$(LDFLAGS_$(BS_LINKER))) $(LDFLAGS_$(BS_TOOLCHAIN))
else
    LDFLAGS += $(LDFLAGS_$(BS_LINKER)) $(LDFLAGS_$(BS_TOOLCHAIN))
endif

DEP_CFLAGS = $(DEP_CFLAGS_GCC)
DEP_ASFLAGS = $(DEP_ASFLAGS_GCC)
BUILTIN_LIBS = $(BUILTIN_LIBS_$(BS_LINKER))

CFLAGS += $(CFLAGS_$(BS_COMPILER))

ifeq ($(BS_LINKER),ARM)
    OCFLAGS += --bin --output
else
    OCFLAGS += -O binary
endif

#
# Variables for targets
#
OBJECTS := $(addprefix $(OBJ_DIR)/, \
           $(patsubst %.S,%.o, \
           $(patsubst %.s,%.o, \
           $(patsubst %.c,%.o, \
           $(patsubst $(BUILD_FIRMWARE_DIR)%,%, \
           $(SOURCES))))))

#
# Module code generation
#
.PHONY: gen_module
gen_module: $(TOOLS_DIR)/gen_module_code.py | $(BUILD_FIRMWARE_DIR)/
	$(TOOLS_DIR)/gen_module_code.py --path $(BUILD_FIRMWARE_DIR) \
	    $(FIRMWARE_MODULES_LIST)

# Include BUILD_FIRMWARE_DIR in the compilation
export INCLUDES += $(BUILD_FIRMWARE_DIR)

#
# Targets
#

.SECONDEXPANSION:

.PHONY: $(LIB_TARGETS_y)
$(LIB_TARGETS_y):
	$(MAKE) -C $@

$(LIB): $(OBJECTS) | $$(@D)/
	$(call show-action,AR,$@)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

$(OBJ_DIR)/%.o: %.c $(EXTRA_DEP) | $$(@D)/
	$(call show-action,CC,$<)
	$(CC) -c $(CFLAGS) $(DEP_CFLAGS)  $< -o $@

$(OBJ_DIR)/%.o: $(BUILD_FIRMWARE_DIR)%.c $(EXTRA_DEP) | $$(@D)/
	$(call show-action,CC,$<)
	$(CC) -c $(CFLAGS) $(DEP_CFLAGS)  $< -o $@

$(OBJ_DIR)/%.o: %.s | $$(@D)/
	$(call show-action,AS,$<)
	$(AS) -c $(ASFLAGS) $(DEP_CFLAGS) $< -o $@

$(OBJ_DIR)/%.o: %.S | $$(@D)/
	$(call show-action,AS,$<)
	$(AS) -c $(CFLAGS) $(DEP_CFLAGS) $(DEP_ASFLAGS) $< -o $@

$(BUILD_PATH)%/:
	$(call show-action,MD,$@)
	$(MD) $@

# Include dependency files
-include $(OBJECTS:%.o=%.d)

endif
