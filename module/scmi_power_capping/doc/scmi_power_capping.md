\ingroup GroupModules Modules
\defgroup GroupSCMI_POWER_CAPPING SCMI power capping protocol

# SCMI power capping protocol implementation

Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.

## Overview

This is the SCMI power capping protocol implementation layer. It supports
both standard protocol commands (excluding fast channel commands) and fast
channels commands. Both types of commands can be configured as per the
following configurations only.

## Configuration

There are three possible configurations which are:

1) Only standard protocol commands are only enabled.
    This option can be enabled by defining the build flag:
    BUILD_HAS_SCMI_POWER_CAPPING_STD_COMMANDS.
2) Only fast channels commands are only enabled.
    This option can be enabled by defining the build flag:
    BUILD_HAS_SCMI_POWER_CAPPING_FAST_CHANNELS_COMMANDS.
3) Both standard protocol commands and fast channels commands are enabled.
    This is enabled by defining the build flags:
    BUILD_HAS_SCMI_POWER_CAPPING_STD_COMMANDS and
    BUILD_HAS_SCMI_POWER_CAPPING_FAST_CHANNELS_COMMANDS.
