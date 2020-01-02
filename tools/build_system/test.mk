#
# Arm SCP/MCP Software
# Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifndef BS_TEST_MK
BS_TEST_MK := 1

# Include folders
INCLUDES := $(FWK_DIR)/include
INCLUDES += $(FWK_DIR)/src
INCLUDES += $(FWK_DIR)/test
INCLUDES += $(OS_DIR)/Include
INCLUDES += $(OS_DIR)/RTX/Include

# Default C flags
CFLAGS += -g3
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -Wno-missing-field-initializers
CFLAGS += -Wno-unused-parameter
CFLAGS += -Wno-strict-aliasing
CFLAGS += -std=c11
CFLAGS += $(addprefix -I,$(INCLUDES))

DEFINES += BUILD_MODE_DEBUG
DEFINES += BUILD_TESTS

# Search path for C sources
VPATH := $(FWK_DIR)/test:$(FWK_DIR)/src

# Define test directory
TEST_DIR := $(BUILD_DIR)/test

# Create list of test targets
TEST_TARGETS = $(addprefix $(TEST_DIR)/, $(TESTS))

# Import utilities
include $(BS_DIR)/defs.mk

# Template for the test rule
define TEST_template
$(1)_LDFLAGS := $(addprefix -Wl$(comma)-wrap=, $($(1)_WRAP))
$(1)_OBJ := $(patsubst %.c, $(TEST_DIR)/%.o, $(COMMON_SRC) $($(1)_SRC))

.PRECIOUS: $(TEST_DIR)/$(1)
$(TEST_DIR)/$(1): $$($(1)_OBJ) | $(TEST_DIR)
	$$(call show-action,LD,$$@)
	$$(CC) -o $$@ $$^ $$($(1)_LDFLAGS)
	$$(call show-action,RUN,$$@)
	$$@
endef

# Create rules for each test
$(foreach test,$(TESTS),$(eval $(call TEST_template,$(test))))

all: $(TEST_TARGETS)

# Create output directory
$(TEST_DIR):
	$(call show-action,MD,$@)
	$(MD) $@

# Build C files
$(TEST_DIR)/%.o: %.c | $(TEST_DIR)
	$(call show-action,CC,$@)
	$(CC) $(CFLAGS) $^ -c -o $@

endif
