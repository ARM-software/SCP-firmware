\ingroup GroupModules Modules
\defgroup GroupMPMM HAL

Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.

MPMM Module Description
=======================

# Overview {#module_mpmm_overview}

This module provides the MPMM (Maximum Power Mitigation mechanism) algorithm.
This mechanism detects and limits high activity events to assist in processor
power domain dynamic power budgeting and limit the triggering of whole-rail
responses to overcurrent conditions.

The algorithm will monitor MPMM threshold counters and controls the performance
level and thresholds. Metrics are provided for each domain so the SCP can use it
to budget the power by limiting the number of cores that can execute higher
activity workloads and switching to a different DVFS operating point.

# MPMM design {#module_mpmm_design}


Each set of cores supporting MPMM and supplied by a single rail is represented
by a domain. A core inside the domain is handled as sub-element. For each core,
there is a set of MPMM counters and threshold registers. The algorithm is split
into two parts. The first part evaluates each core state individually. The
second part evaluates the performance requests for all cores within one domain.
The correct threshold settings are then applied and the new performance limits
are requested.
One special case when a core running the highest MPMM threshold is also
requesting the highest performance level. In this case, the performance is
biased towards this core.

# MPMM configuration {#module_mpmm_configuration}

To use this module the platform code needs to provide the following
configuration options:

## Performance Constraint Table (PCT):
This table lists the thresholds map and the performance cap according to the
number of cores online for each domain.

The number of cores online entry represents the number of cores in the power-on
state. The maximum number of cores supported in a domain is 8.
The entries should be provided in a numerically descending order as shown in the
example table below.

Number of performance limits is the number of threshold - performance limit
tuples. There should be one entry for each number of cores online.

The Threshold map is a 32-bit value. Each 4-bits represents the threshold state
of one core. The threshold map is arranged in a numerically descending order and
not in the order of the core indexes. The threshold map entries should be
provided in a numerically descending order at every performance level cap change
step.

Performance limit is the domain performance level cap for each cores states
described by the threshold bitmap.

This table is an example for a 4 cores in a domain and 3 levels of supported
MPMM thresholds:

| Number of cores online | Number of OPP | Threshold bitmap |   perf limit   |
|          :---:         |     :----:    |      :----:      |      :---:     |
|            4           |       3       |      0x2222      |      2000      |
|                        |               |      0x2211      |      2500      |
|                        |               |      0x2110      |      3000      |
|            3           |       2       |       0x222      |      2500      |
|                        |               |       0x110      |      3000      |
|            2           |       1       |        0x22      |      3000      |


## Base Throttling Count (BTC):
BTC represents the maximum steady state MPMM throttling count which a platform
wants to maintain for each core. The SCP will try to find threshold combinations
which keeps the MPMM throttling around this level while trying to maximize
performance. It is a configuration parameter for the platform to tune. If all
the MPMM counters differences are below the BTC the highest threshold is
selected. If one or more of the MPMM counters differences are above the BTC the
next threshold whose counter delta is just above the BTC is selected.

# MPMM platform implementation {#module_mpmm_platform_implementation}

To use the MPMM module the platform code must define a set of functions for each
core according to the hardware implementation. These functions are:

1) `mpmm_core_check_enabled()`: Check if the MPMM throttling and counters for
    a specific core are enabled.
2) `mpmm_core_set_threshold()`: Sets the MPMM threshold for a specific core.
3) `mpmm_core_counter_read()`: Read one counter from a specific core.
