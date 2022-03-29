# TC0 Platform Variants

Copyright (c) 2022, Arm Limited. All rights reserved.


## Overview

Documentation for TC0 platform can be found at [1].

For the purpose of experimenting some of the software features that have been
introduced in SCP-firmware a new variant of TC0 has been created.
The variant(s) can be chosen at build time by adding:

```sh

make -f Makefile.cmake \
    PRODUCT=tc0 \
    MODE=<debug,release> \
    PLATFORM_VARIANT=<0,1>

```


### Variant 0 (Standard build)

The standard build provides all the features described in [1].
For this default variant, it's not required to provide any extra parameters in
the build commands.


### Variant 1 (Power/Performance testing)

This variant adds support for the following software features:
- Traffic Cop
- MPMM (Maximum Power Mitigation Mechanism)
- Thermal Management
and to have the above fully working, the following dependencies are also added:
- Sensor support
- SCMI Performance FastChannels
- Performance Plugins Handler

Since all the above work on power and performance, to ease evaluating the
features, and to show the configurability options, we split the features among
some of the DVFS domains.

Once built, the features above will act as:

+-----------------+-------------------+
| DVFS domain     | Controlled by (*) |
+-----------------+-------------------+
| KLEIN           | TRAFFIC COP       |
|                 | THERMAL MGMT      |
+-----------------+-------------------+
| MATTERHORN      | TRAFFIC COP       |
|                 | THERMAL MGMT      |
+-----------------+-------------------+
| MATTERHORN ELP  | MPMM              |
|                 | THERMAL MGMT      |
+-----------------+-------------------+

(*) This is in addition to the standard SCMI Performance interface commands.


## Limitations

- The "variant" option is available only with the CMake build.
- The Thermal functionality is limited at this time cause the constant
  temperature being sampled.

References:
[1] https://developer.arm.com/tools-and-software/open-source-software/arm-platforms-software/total-compute-solution
