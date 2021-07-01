\ingroup GroupModules Modules
\defgroup GroupSCMI_PERF SCMI Performance Domain Management Protocol

# Performance Plugins Handler Architecture

Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.

## Overview
The performance of a platform with respect to voltage and frequency can be
managed via the SCMI performance protocol.

The ability of the platform to run at the request from the agent may be
subjected to hardware or software limitations. Examples can be electrical
limitations or temperature limitations.

Such limitations can be imposed by implementing new modules in SCP-firmware,
which affect the final performance level that the platform will run in.

Such modules are interposed between the SCMI performance interface module and
the DVFS module. In this manner, the above modules can affect the
final performance level/limits sent to DVFS.

Since there can be many modules that may be affecting the final performance
level, a *scalable* mechanism to insert or remove modules at compile-time is
implemented.

Such modules are called (performance) plugins and the entire machinery of
insertion/removal as well as interaction with SCMI performance is managed by the
*perf_plugins_handler* extension of SCMI performance.

## Architecture

### No plugins

A scenario where a request for a performance limit is not *affected* by any
other performance-related monitor/plugins. Simply scmi_performance module calls
DVFS HAL.

    +-----------+                                      +-----------+
    |           +------------------------------------->+           |
    | scmi-perf +                                      +   DVFS    |
    |           +<-------------------------------------+           |
    +-----------+                                      +-----------+


### With (performance) plugins

In a scenario where some additional modules need to affect the performance
limits, these are called prior to submit the request to DVFS. The plugins
handler calls each of the plugins with the information regarding the performance
request. In the same call, plugins can adjust the performance request according
to their internal algorithms.

Once all the plugins have been called, the plugins handler will provide SCMI
performance with the adjusted performance limits which, in turn, will submit the
request to DVFS accordingly.

    +-----------+                                      +-----------+
    |           +<------------------------------------>+           |
    | scmi-perf +<--+                                  |   DVFS    |
    |           |   |                                  |           |
    +-----+- - -+---v-+         +-----------+          +-----------+
          |           |         |           |
          |  plugins  +<------->+  plugin x |
          |  handler  +<---+    |           |
          +-----------+    |    +-----------+
                           |    +-----------+
                           |    |           |
                           +--->+  plugin y |
                                |           |
                                +-----------+

The machinery relies on SCMI FastChannels as a source of performance requests
and for its periodic polling. At every polling, the performance plugins are
given the performance values and the adjusted values are returned on the same
call via a data structure.

Plugins can take advantage of this regular call for their internal algorithms.
Note that there will always be a periodic call, regardless whether or not there
is a "new" performance request in the FastChannels.

### Asynchronous calls

In some circumstances a plugin may needs to set a limit to the performance
driver before waiting for the next tick. This can be done by using the provided
perf_plugins_handler_api.
This interface should be used only when the limit set cannot wait till the next
periodic call, because of a significant performance degradation.
A plugin should use the regular update call to affect the performance state
whenever possible.

### Min/Max limits policy

The final values that will be transferred to DVFS are decided with the following
rule:

    final_perf_max_limit = MIN(agent_requested_limit, plugin_adjusted_limit)

    final_perf_min_limit = MAX(agent_requested_limit, plugin_adjusted_limit)

### Domain aggregation (physical/logical)

Plugins_handler also implements a basic form of domain aggregation to support
situations where the performance domains exposed to OS through SCMI are
different from those that are available in the platform. These are respectively
logical (performance control) and physical (aka dependency domain).

A typical example is a platform where SCP exposes per-cpu controls through SCMI
Performance while having fewer physical domains (DVFS on a cluster-basis or a
group of CPUs for example). In this case, there is a need to collect and combine
the different requests for the logical domains into a final single value that is
the one to be applied to the physical domain. This aggregation is performed by
the plugins handler.

In the platform configuration, a plugin can choose whether its view is on
logical or physical domains.
A plugin is updated when the data for all the logical domains, within the same
dependency domain, is collected.
This way the plugins will have a snapshot of the performance requests for all
the logical domains within a physical domain, and the domain identifier provided
is as follow:
    FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_SCMI_PERF,
        physical_domain, last_logical_domain)

For plugins whose view is on physical domains, requests coming from logical
domains will need to appear as they came from a single physical domain (if
logical domains are present), and this is done as part of the aggregation
policy. Those plugins will receive an update with an aggregated value and the
domain identifier is as above. In this case, the sub_element index can be
ignored.

    Example: Aggregation of logical values

    +------------+-----------+-----------+-----------+  +------------------+
    |            | log dom 0 | log dom 1 |  phy dom  |  |  plugin receives |
    +------------------------------------------------+  +------------------+
    | limit_max  |   1000    |    950    |    950    |  |    950           |
    +------------------------------------------------+  +------------------+
    | limit_min  |   200     |    100    |    200    |  |    200           |
    +------------+-----------+-----------+-----------+  +------------------+

Then the same Min/Max policy described above applies when comparing values from
OS and adjusted values from a plugin.

NOTE: The plugins handler can handle the domain aggregation without the plugins.
If only domain aggregation is required, there is no need to have performance
plugins.

### Data exchange

Each plugin will receive a snapshot of the performance domain level and limits
values at every tick (SCMI FastChannels) and it can modify the limits if it
wishes to affect the original request coming from the SCMI interface. It is
optional for the plugin to update the values.

It is also possible for a plugin to receive a full snapshot of the performance
level and limits for all the performance domains. This can be chosen in the
module configuration. In this case, a plugin will be called once at every
tick period.

Possible configurations & data received:

    +----------------------+------------------------------------------------+
    | config->dom_type     | data received                                  |
    | (plugin_domain_type) | (perf_plugins_perf_update)                     |
    +----------------------+------------------------------------------------+
    | _TYPE_PHYSICAL       | Single physical domain.                        |
    |                      | A plugin gets a call for each physical domain  |
    |                      | every tick period.                             |
    |                      |                                                |
    |                      | Example, 3 physical domains:                   |
    |                      | A plugin which implements the `update` function|
    |                      | will be called 3 times (at every tick period): |
    |                      | 1st call with data for domain 0                |
    |                      | 2nd call with data for domain 1                |
    |                      | 3rd call with data for domain 2                |
    +----------------------+------------------------------------------------+
    | _TYPE_LOGICAL        | Array of logical domains (within same physical)|
    |                      | A plugin gets a call for each set of logical   |
    |                      | domains every tick period.                     |
    |                      |                                                |
    |                      | Example, 1 physical domain with 3 logical      |
    |                      | domains:                                       |
    |                      | A plugin which implements the `update` function|
    |                      | will be called 1 time (at every tick period):  |
    |                      | 1st call with data array for logical domains 0,|
    +----------------------+------------------------------------------------+
    | _TYPE_FULL           | Array of physical domains                      |
    |                      | A plugin gets called once for all the physical |
    |                      | domains every tick period.                     |
    |                      |                                                |
    |                      | Example, 3 physical domains:                   |
    |                      | A plugin which implements the `update` function|
    |                      | will be called once (at every tick period):    |
    |                      | 1st call with data array for domain 0, 1 and 2 |
    +----------------------+------------------------------------------------+


## Use

Each plugin is expected to implement the update() function as specified in the
perf_plugins_api documentation.
They are also supposed to allow the scmi_performance module to bind to them and
allow the above API to be bound.

The whole feature is enabled by BS_FIRMWARE_HAS_PERF_PLUGIN_HANDLER (make) or
SCP_ENABLE_PLUGIN_HANDLER_INIT (CMake).

## Restrictions
- A platform wishing to use the plugins handler, needs to have FastChannels
implemented (and enabled).
- This entire mechanism for adding/removing plugins is not supported with the
traditional SCMI SMT/doorbell transport.
- The order in which the plugins are listed in the plugins table (module's
configuration )is the order in which they will be called by the plugins handler.
However, plugins whose view is _TYPE_FULL should be placed as last entries in
the table of plugins in the module's configuration. This is to ensure that all
the other plugins have run before a consolidated full picture of the
level/limits can be shared with the remaining plugins.

## Configuration Example 1 (plugin with physical/DVFS domains view)

    static const struct mod_scmi_perf_domain_config domains[] = {
        [DVFS_DOMAIN_0] = {
            .fast_channels_addr_scp = (uint64_t[]) { ... },
            .fast_channels_addr_ap = (uint64_t[]) { ... },
        },
        ...
        [DVFS_DOMAIN_n] = {
            .fast_channels_addr_scp = (uint64_t[]) { ... },
            .fast_channels_addr_ap = (uint64_t[]) { ... },
        },
    };

    static const struct mod_scmi_plugin_config plugins_table[] = {
        [0] = {
            .id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_<PLUGIN>),
            .dom_type = PERF_PLUGIN_DOM_TYPE_PHYSICAL,
        },
    };

    struct fwk_module_config config_scmi_perf = {
        .data = &((struct mod_scmi_perf_config){
            ...
            .plugins = plugins_table,
            .plugins_count = FWK_ARRAY_SIZE(plugins_table)
        }),
    };

## Configuration Example 2 (plugin with logical/SCMI domains view, 2 logical
    domains, 1 DVFS domain)

    static const struct mod_scmi_perf_domain_config domains[] = {
        [SCMI_PERF_DOMAIN_0] = {
            .fast_channels_addr_scp = (uint64_t[]) { ... },
            .fast_channels_addr_ap = (uint64_t[]) { ... },
            .phy_group_id =
                FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0),
        },
        ...
        [SCMI_PERF_DOMAIN_1] = {
            .fast_channels_addr_scp = (uint64_t[]) { ... },
            .fast_channels_addr_ap = (uint64_t[]) { ... },
            .phy_group_id =
                FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0),
        },
    };

    static const struct mod_scmi_plugin_config plugins_table[] = {
        [0] = {
            .id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_<PLUGIN>),
            .dom_type = PERF_PLUGIN_DOM_TYPE_LOGICAL,
        },
    };

    struct fwk_module_config config_scmi_perf = {
        .data = &((struct mod_scmi_perf_config){
            ...
            .plugins = plugins_table,
            .plugins_count = FWK_ARRAY_SIZE(plugins_table)
        }),
    };

Note that a plugin in this case can choose either logical or physical view.

## Configuration Example 3 (no plugins, 2 logical domains, 1 DVFS domain)

    static const struct mod_scmi_perf_domain_config domains[] = {
        [SCMI_PERF_DOMAIN_0] = {
            .fast_channels_addr_scp = (uint64_t[]) { ... },
            .fast_channels_addr_ap = (uint64_t[]) { ... },
            .phy_group_id =
                FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0),
        },
        ...
        [SCMI_PERF_DOMAIN_1] = {
            .fast_channels_addr_scp = (uint64_t[]) { ... },
            .fast_channels_addr_ap = (uint64_t[]) { ... },
            .phy_group_id =
                FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0),
        },
    };

This example is for cases when only the logical domains aggregation policy is
required.

## Configuration Example 4 (2 plugins with physical domain view)

    static const struct mod_scmi_perf_domain_config domains[] = {
        [DVFS_DOMAIN_0] = {
            .fast_channels_addr_scp = (uint64_t[]) { ... },
            .fast_channels_addr_ap = (uint64_t[]) { ... },
        },
    };

    static const struct mod_scmi_plugin_config plugins_table[] = {
        [0] = {
            .id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_<PLUGIN>),
            .dom_type = PERF_PLUGIN_DOM_TYPE_PHYSICAL,
        },
        [1] = {
            .id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_<PLUGIN>),
            .dom_type = PERF_PLUGIN_DOM_TYPE_PHYSICAL,
        },
    };

    struct fwk_module_config config_scmi_perf = {
        .data = &((struct mod_scmi_perf_config){
            ...
            .plugins = plugins_table,
            .plugins_count = FWK_ARRAY_SIZE(plugins_table)
        }),
    };

## Configuration Example 5 (3 plugins with different domain view)

    static const struct mod_scmi_perf_domain_config domains[] = {
        [DVFS_DOMAIN_0] = {
            .fast_channels_addr_scp = (uint64_t[]) { ... },
            .fast_channels_addr_ap = (uint64_t[]) { ... },
        },
        ...
        [DVFS_DOMAIN_n] = { ... },
    };

    static const struct mod_scmi_plugin_config plugins_table[] = {
        [0] = {
            .id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_<PLUGIN0>),
            .dom_type = PERF_PLUGIN_DOM_TYPE_<PHYSICAL/LOGICAL>,
        },
        [1] = {
            .id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_<PLUGIN1>),
            .dom_type = PERF_PLUGIN_DOM_TYPE_<PHYSICAL/LOGICAL>,
        },
        /*
         * This plugin id needs to be placed as last entry because its view is
         * _TYPE_FULL (see restrictions section).
         */
        [2] = {
            .id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_<PLUGIN2>),
            .dom_type = PERF_PLUGIN_DOM_TYPE_FULL,
        },
    };

    struct fwk_module_config config_scmi_perf = {
        .data = &((struct mod_scmi_perf_config){
            ...
            .plugins = plugins_table,
            .plugins_count = FWK_ARRAY_SIZE(plugins_table)
        }),
    };

NOTE: In this example no.5 the order of plugin callbacks is the following:
- plugin 0 with physical/logical domain 0
- plugin 1 with physical/logical domain 0
- plugin 0 with physical/logical domain n
- plugin 1 with physical/logical domain n
- plugin 2 with physical/logical all domains
