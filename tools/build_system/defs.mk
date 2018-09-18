#
# Arm SCP/MCP Software
# Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Escape comma character
comma = ,

#
# Suffix used for directories containing resources built with multithreading
# support enabled
MULTHREADING_SUFFIX := _mt

#
# Suffix used for directories containing resources built with notification
# support enabled
NOTIFICATION_SUFFIX := _nt

#
# SCP/MCP Software Version
#
$(call add_once,DEFINES,BUILD_VERSION_MAJOR=$(VERSION_MAJOR))
$(call add_once,DEFINES,BUILD_VERSION_MINOR=$(VERSION_MINOR))
$(call add_once,DEFINES,BUILD_VERSION_PATCH=$(VERSION_PATCH))
$(call add_once,DEFINES,BUILD_VERSION_STRING=\"$(VERSION_STRING)\")
$(call add_once,DEFINES, \
    BUILD_VERSION_DESCRIBE_STRING=\"$(VERSION_DESCRIBE_STRING)\")

$(call add_once,DEFINES,RTX_NO_MULTITHREAD_CLIB)

#
# Returns a path relative to the top directory
#
# Param $1 Target path
# Return String containing the relative path
define relative-path
	$(subst $(TOP_DIR)/,,$1)
endef

#
# Shows an action that is being performed on a path
#
# Param $1 Action string
# Return None.
define show-action
	@echo [$1] $(call relative-path, $2)
endef

#
# Returns the path to the library binary
#
# Param $1 Library name
# Return String containing the library path
define lib_path
    $(BUILD_FIRMWARE_DIR)/$1/$(MODE)/lib/lib.a
endef

#
# Adds a value to a list variable only once.
#
# Param $1 Variable name to receive the value
# Param $2 Value to be added to the list. If the value is already in the list
#   then no action is performed.
# Return None.
define add_once
    $(eval $(call _add_once,$1,$2))
endef

define _add_once
    ifeq ($$(findstring $2,$($1)),)
        $1 += $2
    endif
endef

#
# Converts input string to uppercase
#
# Param $1 String
# Return Input string in uppercase
define to_upper
$(shell echo $(1) | tr '[:lower:]' '[:upper:]')
endef
