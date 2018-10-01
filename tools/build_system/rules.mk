#
# Arm SCP/MCP Software
# Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include $(BS_DIR)/defs.mk
include $(BS_DIR)/cpu.mk

ifeq ($(BUILD_HAS_MULTITHREADING),yes)
    # Add the OS directory to the main INCLUDES list
    INCLUDES += $(OS_DIR)/Include
    DEFINES += BUILD_HAS_MULTITHREADING
endif

ifeq ($(BUILD_HAS_NOTIFICATION),yes)
    DEFINES += BUILD_HAS_NOTIFICATION
endif

export AS := $(CC)
export LD := $(CC)

include $(BS_DIR)/toolchain.mk

ifeq ($(BS_LINKER),ARM)
    export AR := $(shell $(CC) --print-prog-name armar)
    export OBJCOPY := $(shell $(CC) --print-prog-name fromelf)
    export SIZE := $(shell $(CC) --print-prog-name size)
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
    CFLAGS_GCC  += -mtune=native
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

    CFLAGS_GCC  += -mcpu=$(BS_ARCH_CPU)
    ASFLAGS_GCC += -mcpu=$(BS_ARCH_CPU)
    LDFLAGS_GCC += -mcpu=$(BS_ARCH_CPU)
    LDFLAGS_ARM += -mcpu=$(BS_ARCH_CPU)

    # Optional ISA ("sub-arch") parameter
    ifneq ($(BS_ARCH_ISA),)
        CFLAGS_GCC  += -m$(BS_ARCH_ISA)
        ASFLAGS_GCC += -m$(BS_ARCH_ISA)
        LDFLAGS_GCC += -m$(BS_ARCH_ISA)
        LDFLAGS_ARM += -m$(BS_ARCH_ISA)
    endif
endif

#
# Warning flags
#

CFLAGS_GCC += -Werror
CFLAGS_GCC += -Wall
CFLAGS_GCC += -Wextra
CFLAGS_GCC += -pedantic
CFLAGS_GCC += -pedantic-errors

CFLAGS_GCC += -Wno-unused-parameter

# GCC is not currently consistent in how it applies this warning, but this flag
# should be removed should we move to a version that can build the firmware
# without it.
CFLAGS_GCC += -Wno-missing-field-initializers

# Clang picks up a number of situations that GCC does not with this warning
# enabled. Most of them do not have easy fixes, and are valid C, so this flag
# should remain unless we move to a version of Clang/Arm Compiler that does not
# warn about the situations that have not already been fixed.
CFLAGS_CLANG += -Wno-missing-braces

CFLAGS_GCC += -g
CFLAGS_GCC += -std=c11
CFLAGS_CLANG += -fshort-enums # Required by RTX

CFLAGS_GCC += -fno-exceptions

DEP_CFLAGS_GCC = -MD -MP

ASFLAGS_GCC += -x
ASFLAGS_GCC += assembler-with-cpp
ASFLAGS_GCC += -g

ARFLAGS_GCC = -rc

LDFLAGS_GCC += -Wl,--cref

# Force an undefined reference to the exceptions table so that it is included
# even if no code refers to it.
LDFLAGS_GCC += -Wl,--undefined=exceptions
LDFLAGS_ARM += -Wl,--undefined=exceptions

# Ensure main() is not removed by the linker
LDFLAGS_ARM += -Wl,--undefined=main

BUILTIN_LIBS_GCC := -lc -lgcc

ifeq ($(MODE),release)
    O ?= $(DEFAULT_OPT_GCC_RELEASE)

    # Disable assertions in release mode
    DEFINES += NDEBUG

else
    O ?= $(DEFAULT_OPT_GCC_DEBUG)

    DEFINES += BUILD_MODE_DEBUG
endif

#
# Always include the framework library
#
INCLUDES += $(FWK_DIR)/include

#
# Toolchain-independent flags
#
CFLAGS += -O$(O)
CFLAGS += $(addprefix -I,$(INCLUDES)) $(addprefix -D,$(DEFINES))
ASFLAGS += $(addprefix -I,$(INCLUDES)) $(addprefix -D,$(DEFINES))

#
# Assign toolchain-specific flags
#
CFLAGS += $(CFLAGS_GCC)
ASFLAGS += $(ASFLAGS_GCC)
ARFLAGS = $(ARFLAGS_GCC)
LDFLAGS += $(LDFLAGS_$(BS_LINKER))
DEP_CFLAGS = $(DEP_CFLAGS_GCC)
BUILTIN_LIBS = $(BUILTIN_LIBS_$(BS_LINKER))

ifeq ($(BS_COMPILER),CLANG)
    CFLAGS += $(CFLAGS_CLANG)
endif

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
           $(SOURCES)))))

GENERATED_DIRS := $(sort $(BUILD_DIR) \
                         $(BUILD_DOC_DIR) \
                         $(BUILD_FIRMWARE_DIR) \
                         $(dir $(LIB) \
                               $(OBJECTS) \
                               $(SCATTER_PP) \
                               $(SCATTER_SRC) \
                               $(TARGET_BIN)))

#
# Module code generation
#
.PHONY: gen_module
gen_module: $(TOOLS_DIR)/gen_module_code.py $(BUILD_FIRMWARE_DIR)
	$(TOOLS_DIR)/gen_module_code.py --path $(BUILD_FIRMWARE_DIR) $(FIRMWARE_MODULES_LIST)

# Include BUILD_FIRMWARE_DIR in the compilation
export INCLUDES += $(BUILD_FIRMWARE_DIR)

#
# Targets
#

.PHONY: $(LIB_TARGETS_y)
$(LIB_TARGETS_y):
	$(MAKE) -C $@

$(LIB): $(OBJECTS) | $(targetdir)
	$(call show-action,AR,$@)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

$(OBJ_DIR)/%.o: %.c $(EXTRA_DEP) | $(targetdir)
	$(call show-action,CC,$<)
	$(CC) -c $(CFLAGS) $(DEP_CFLAGS)  $< -o $@

$(OBJ_DIR)/%.o: %.s | $(targetdir)
	$(call show-action,AS,$<)
	$(AS) -c $(ASFLAGS) $(DEP_CFLAGS) $< -o $@

$(OBJ_DIR)/%.o: %.S | $(targetdir)
	$(call show-action,AS,$<)
	$(AS) -c $(CFLAGS) $(DEP_CFLAGS) $< -o $@

$(GENERATED_DIRS):
	$(call show-action,MD,$@)
	$(MD) $@

.PHONY: doc
doc: | $(BUILD_DOC_DIR)
	$(call show-action,DOC,)
	$(DOC) $(DOC_DIR)/config.dxy

# Include dependency files
-include $(OBJECTS:%.o=%.d)
