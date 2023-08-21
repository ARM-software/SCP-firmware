\ingroup GroupModules Modules
\defgroup GroupMHUv3 Message Handling Unit (MHU) v3 Driver

# MHU Hardware version 3 driver


Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.

## Overview

This is a driver module that supports MHU hardware version 3.

Although, MHUv3 specifications specify various extensions, at the moment, only
the following extensions are supported by this driver:

1. Doorbell Channel extension
2. Fast Channel extension

### Limitations

Doorbell channel implementation is limited to 32 doorbell channels for
efficiency reasons.

However, this can be extended to support more on a larger system with more
powerful processors. In general, although MHUv3 specification allows up to 128
channels, on a practical system for a SCP use case, the number of doorbell
channels is expected to remain < 32.
