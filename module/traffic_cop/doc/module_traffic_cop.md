\ingroup GroupModules Modules
\defgroup GroupTrafficCop Traffic Cop

Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.

# Traffic Cop

## Overview
Traffic Cop is a performance plugin module. It imposes limitations on
performance according to the platform electric capabilities.

The limitation can be configured based on the number of active cores for each
domain. In this case, a domain could be a group of cores supplied by the same
power rail. The configuration is a simple lookup table that maps the number of
active cores and the maximum performance level allowed as shown here. This table
is called the Performance Constraint Table or PCT.

This PCT table is an example of 4 cores domain:

| Number of cores online |   perf limit   |
|          :---:         |      :---:     |
|            4           |      2000      |
|            3           |      2500      |
|            2           |      3000      |


## Configuration Example

```config_traffic_cop.c```

```C
static struct mod_tcop_pct_table pct[] = {
    { .cores_online = 4,
      .perf_limit =  2000,
    },
    { .cores_online = 3,
      .perf_limit =  2500,
    },
    { .cores_online = 2,
      .perf_limit =  3000,
    },
    { 0 },
};
```

NOTE: The table must be provided in descending order, starting with the highest
number of cores allowed.
