\ingroup GroupModules Modules
\defgroup GroupMPMM Max Power Mitigation Mechanism (MPMM)

Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.

MPMM Module Description
=======================

# Overview {#module_mpmm_overview}

This module provides the MPMM (Maximum Power Mitigation Mechanism) algorithm.
This mechanism detects and limits high activity events to assist in processor
power domain dynamic power budgeting and limit the triggering of whole-rail
responses to overcurrent conditions.

The algorithm will monitor MPMM threshold counters and controls the performance
limits and thresholds. Metrics are provided for each domain so the SCP can use
it to budget the power by limiting the number of cores that can execute higher
activity workloads and switching to a different DVFS operating point.

# MPMM design {#module_mpmm_design}

Each set of cores supporting MPMM and supplied by a single rail is represented
by a domain. A core inside the domain is handled as sub-element. For each core,
there is a set of MPMM counters and threshold registers. The algorithm is split
into two parts. The first part evaluates each core state individually. The
second part evaluates the performance requests for all cores within one domain.
The correct threshold settings for each core are then applied and the new
performance limits are requested.

# MPMM configuration {#module_mpmm_configuration}

To use this module the platform code needs to provide the following
configuration options:

## Performance Constraint Table (PCT):
This table lists the thresholds map and the performance limits according to the
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
provided in a numerically descending order at every performance limit change
step. The number of table entries and the exact values should be populated
according to the hardware implementation.

Performance limit is the domain performance level limit for each cores states
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
wants to maintain for each core. The algorithm will try to find threshold
combinations which keeps the MPMM throttling around this level while trying to
maximize performance.
It is a configuration parameter for the platform to tune. If all the MPMM
counters differences are below the BTC the highest threshold is selected. If one
or more of the MPMM counters differences are above the BTC the next threshold
whose counter delta is just above the BTC is selected.
