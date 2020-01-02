#
# Arm SCP/MCP Software
# Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Description:
#     Makefile for top-level documentation.
#

MODULE_INCLUDES += $(shell ls -d $(MODULES_DIR)/*/include/ 2>/dev/null)
MODULE_INCLUDES += $(shell ls -d $(PRODUCTS_DIR)/*/module/*/include/ \
                                 2>/dev/null)

MODULE_DOCS += $(shell ls -d $(MODULES_DIR)/*/doc/ 2>/dev/null)
MODULE_DOCS += $(shell ls -d $(PRODUCTS_DIR)/*/module/*/doc/ 2>/dev/null)

export MODULE_INCLUDES
export MODULE_DOCS

include $(BS_DIR)/rules.mk

.PHONY: doc
doc: | $(BUILD_DOC_DIR)/
	$(call show-action,DOC,)
	$(DOC) $(DOC_DIR)/config.dxy
