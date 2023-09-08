\ingroup GroupModules Modules
\defgroup GroupThermal Thermal Management

# Thermal Management Architecture

Copyright (c) 2022, Arm Limited. All rights reserved.


## Overview

Thermal Management is a basic closed-loop temperature controller which
dynamically controls the platform performance in a thermal envelope.

With a closed control loop and by intelligently dividing the power among actors,
Thermal Management can efficiently distribute the power within the thermal
constraints.

The power delivered to each actor (CPU, GPU, etc) is controlled by adjusting the
frequency and the voltage provided to such actors (performance limits).
Each actor can be assigned with its own power model, which defines the equations
for converting power into performance levels and vice-versa.

By allocating the correct performance level, this is reflected to the correct
power consumed and thus the temperature is maintained at the desired level.

In a system, multiple Thermal Management controllers could exist. Each of them
is ruled by different temperature domains where they have their own dedicate
temperature sensor.

## Architecture of a Thermal Management controller

The main two blocks composing Thermal Mgmt are the PI (Proportional and
Integral) control loop and the power divider.

                               Thermal Design
    current temp                Power (TDP)      weight (config)
         |                           |             |   |
         |                           |             |   |
         |                           |             |   |
         v                           v             v   v
       +-+-+     +---------+       +-+-+        +--+---+--+
       |   +---->+ PI Ctrl +------>+   +------->+  power  +----> power granted
       +-+-+     +---------+       +---+   +--->+ divider +---->
         ^                                 |    +--+---+--+
         |                                 |       ^   ^
         |    performance request ---------+       |   |
         |                                   +-----+   +-----+
    control temp                             |               |
                                             v               v
                                         +---+---+       +---+---+
                                         | power |       | power |
                                         | model |       | model |
                                         +-------+       +-------+


The control loop reacts to temperature deviations from the control settings and
provides a (proportional and integral) signal which is then converted into
allocatable power offset.
This available power is then distributed across the actors by a basic weighted
bias mechanism. Each actor will get a fraction of the available power
proportionally to the bias.

Thermal Management runs internally on two loops: fast loop and slow loop.
The fast loop is required to provide a fast adjustment of the performance
requests, while the slow loop is meant to update the PI control (which in turns
provides an updated allocatable power). The temperature is
sampled at slow loop cadence.

Thermal Management is taking advantage of the plugin-handler extension to get a
synchronous tick with the performance requests coming from SCMI. This way the
fast loop is directly sync'ed with the performance chain to adjust the
performance limits. This tick imposes the fast loop periodicity.
The slow loop is derived internally from the fast loop cadence by a configurable
multiplication factor. This will allow custom tuning of the PI control timings.

An additional module plays an important role in the entire algorithm for
translating the power to performance and vice-versa. It is required that each
actor has got their own power model which will be implemented as an additional
platform-specific module to achieve best modularity and flexibility.

The interface for the power model is defined by Thermal Management
(mod_thermal_power_model_api).


## Algorithm

At each regular tick (fast loop), the Thermal Management will:
- convert the requested performance level into power
- attempt to distribute the power across actors based on their request and
  weights. Any spare power is collected or any shortages kept track of.
- any carry-over power not consumed in the previous cycle is added onto the
  available spare power
- re-distribute any available spare power across actors based on their shortage
- any spare power left becomes the carry-over power for the next cycle
- convert the granted power into requested performance level
- apply a performance limit on the actor's corresponding domain if the power
  requested could not be met

The above conversions power<->performance are performed within the
platform-specific power model module.

With a slower periodicity (slow loop), the Thermal Management will:
- initiate a temperature reading
- run the PI control and update the total available power


## Use

To use the Thermal Management the following dependencies are required:
- performance plugin handler enabled (BS_FIRMWARE_HAS_PERF_PLUGIN_HANDLER)
- Power Model in product/<product-name>/module/product_power_model


## Limitations

Currently the implementation is in "prototype" stage and limited tests have been
carried out.


## Tunings & Settings

### Global PI control tunings

`slow_loop_mult`
Multiplier applied to the the base tick via the ->update callback.
For example: if the tick period is 5ms, then a value of 20 will give the PI
control a refresh rate at 100ms (= 5 * 20).

### Per-temperature domain tunings

`tdp (thermal design power)`
The thermal design power for all the actors monitored. This can be an abstract
value as long as the power model can work with it.

`switch_on_temperature`
The temperature above which the Thermal Mgmt algorithm will run. The unit needs
to be consistent with the value provided by the targeted temperature sensor.

`cold_state_power`
Power limit used in cold state - temperature is below the switch_on_temperature.

`control_temperature`
The control temperature for the platform. Above this temperature the Thermal
Mgmt will limit the power/performance.

`integral_cutoff`
The error threshold below which the errors are accumulated. This may be useful
to avoid accumulating errors when the error is positive i.e the temperature is
below the control.

`integral_max`
The maximum value of accumulated errors. This may be useful to limit the
positive accumulation of error which, in turn, will affect the overshooting
operation.

`k_p_undershoot`
The proportional coefficient used when the error is positive (actual temperature
below control temperature)

`k_p_overshoot`
The proportional coefficient used when the error is negative (actual temperature
above control temperature)

`k_integral`
The integral coefficient used when multiplying with the accumulated error.

### Per-actor tunings

`weight`
The coefficient used as an allocation factor for a specific actor. Its value can
be "any" and expresses the corresponding weight an actor has over the others.

### Thermal protection

There is the possibility to configure a temperature protection that allows to
configure two different alarms, warning and critical. They can be configured
independently but the critical level should be above the one for warning. A
callback can be configured for each threshold in order for an additional module
to take action to reduce the temperature or initiate a power-down sequence.


## Power models

The power model is a platform-specific module that needs to be implemented by
each platform in order to perform power-to-performance level conversions and
vice-versa.
This allows separation from Thermal Management common-code to platform-specific
characteristics.
When implementing the APIs, the Power Model module should also allow incoming
bind requests from Thermal Mgmt.

## Activity factor
There is the possibility to specify an activity factor API driver functionality
that allows to accumulate idle power from each actor during operation and
distribute it amongst other actors to use it. Activity factor is an optional
feature that can be configured when it is required.
The driver API is platform specific because it gives flexibility in terms of
implementation depending on system configuration.


## Configuration Example 1 (2 actors, 1 temperature domain)

```C
static struct mod_thermal_mgmt_actor_config actor_table_domain0[] = {
    [0] = {
        .driver_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PLAT_POWER_MODEL, 0),
        .dvfs_domain_id =
            FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_ACTOR0),
        .weight = 100,
    },
    [1] = {
        .driver_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PLAT_POWER_MODEL, 1),
        .dvfs_domain_id =
            FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_ACTOR1),
        .weight = 200,
    },
};

struct fwk_element thermal_mgmt_domains_elem_table = {
    [0] = {
        .name = "Thermal Domain 0",
        .data = &((struct mod_thermal_mgmt_dev_config){
            .slow_loop_mult = 20,

            .tdp = 5000,
            .pi_controller = {
                .switch_on_temperature = 50,
                .control_temperature = 60,
                .integral_cutoff = 0,
                .integral_max = 100,
                .k_p_undershoot = 1,
                .k_p_overshoot = 1,
                .k_integral = 1,
            },

            .sensor_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SENSOR, 0),
            .temp_protection = &((struct mod_thermal_mgmt_protection_config){
                .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_, 0),
                .driver_api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_,
                    0),
                .warn_temp_threshold = 60,
                .crit_temp_threshold = 85,
            }),
            .driver_api_id =
                FWK_ID_API_INIT(FWK_MODULE_IDX_PLAT_POWER_MODEL, 0),
            .thermal_actors_table = actor_table_domain0,
            .thermal_actors_count = FWK_ARRAY_SIZE(actor_table_domain0),
        }),
        .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_thermal_mgmt_element_table),
    },
    [1] = { 0 } /* Termination description */
};

static const struct fwk_element *get_thermal_mgmt_element_table(
    fwk_id_t module_id)
{
    return thermal_mgmt_domains_elem_table;
}

struct fwk_module_config config_thermal_mgmt = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};

```

And the power model should implement the following API:

```C

uint32_t plat_level_to_power(fwk_id_t domain_id, const uint32_t level)
{
    /* compute the power for this actor/domain at this level */
    return power;
}

uint32_t plat_power_to_level(fwk_id_t domain_id, const uint32_t power)
{
    /* compute the performance level for this actor/domain for this power */
    return perf_level;
}

struct mod_thermal_mgmt_power_model_api power_model_api = {
    .level_to_power = plat_level_to_power,
    .power_to_level = plat_power_to_level,
};

```

## Configuration Example 2 (2 actors, 1 temperature domain and activity factor)

```C
static struct mod_thermal_mgmt_actor_config actor_table_domain0[] = {
    [0] = {
        .driver_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PLAT_POWER_MODEL, 0),
        .dvfs_domain_id =
            FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_ACTOR0),
        .weight = 100,
        .activity_factor = &((struct mod_thermal_mgmt_activity_factor_config){
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_PLATFORM_ACTIVITY,
                0),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PLATFORM_ACTIVITY,
                MOD_PLATFORM_ACTIVITY_DRIVER_API_IDX),
        }),
    },
    [1] = {
        .driver_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PLAT_POWER_MODEL, 1),
        .dvfs_domain_id =
            FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_ACTOR1),
        .weight = 200,
        .activity_factor = &((struct mod_thermal_mgmt_activity_factor_config){
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_PLATFORM_ACTIVITY,
                1),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PLATFORM_ACTIVITY,
                MOD_PLATFORM_ACTIVITY_DRIVER_API_IDX),
    },
};

struct fwk_element thermal_mgmt_domains_elem_table = {
    [0] = {
        .name = "Thermal Domain 0",
        .data = &((struct mod_thermal_mgmt_dev_config){
            .slow_loop_mult = 20,

            .tdp = 5000,
            .pi_controller = {
                .switch_on_temperature = 50,
                .control_temperature = 60,
                .integral_cutoff = 0,
                .integral_max = 100,
                .k_p_undershoot = 1,
                .k_p_overshoot = 1,
                .k_integral = 1,
            },

            .sensor_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SENSOR, 0),
            .temp_protection = &((struct mod_thermal_mgmt_protection_config){
                .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_, 0),
                .driver_api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_,
                    0),
                .warn_temp_threshold = 60,
                .crit_temp_threshold = 85,
            }),
            .driver_api_id =
                FWK_ID_API_INIT(FWK_MODULE_IDX_PLAT_POWER_MODEL, 0),
            .thermal_actors_table = actor_table_domain0,
            .thermal_actors_count = FWK_ARRAY_SIZE(actor_table_domain0),
        }),
        .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_thermal_mgmt_element_table),
    },
    [1] = { 0 } /* Termination description */
};

static const struct fwk_element *get_thermal_mgmt_element_table(
    fwk_id_t module_id)
{
    return thermal_mgmt_domains_elem_table;
}

struct fwk_module_config config_thermal_mgmt = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};

```

The activity factor module should implement the following API:

```C

int get_activity_factor(fwk_id_t domain_id, uint16_t *activity)
{
    /* Compute performance factor for the given domain id */
    *activity = plat_activity_factor;
    return FWK_SUCCESS;
}

struct mod_thermal_mgmt_activity_factor_api activity_factor_api = {
    .get_activity_factor = plat_get_activity_factor,
};

```

## Configuration Example 3 (thermal protection)

There is the possibility to only configure the module as a thermal protection.

```C
struct fwk_element thermal_mgmt_domains_elem_table = {
    [0] = {
        .name = "Thermal Domain 0",
        .data = &((struct mod_thermal_mgmt_dev_config){
            .slow_loop_mult = 5,
            .sensor_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SENSOR, 0),
            .temp_protection = &((struct mod_thermal_mgmt_protection_config){
                .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_, 0),
                .driver_api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_,
                    0),
                .warn_temp_threshold = 60,
                .crit_temp_threshold = 85,
            }),
        }),
        .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_thermal_mgmt_element_table),
    },
    [1] = { 0 } /* Termination description */
};

static const struct fwk_element *get_thermal_mgmt_element_table(
    fwk_id_t module_id)
{
    return thermal_mgmt_domains_elem_table;
}

struct fwk_module_config config_thermal_mgmt = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};

```
