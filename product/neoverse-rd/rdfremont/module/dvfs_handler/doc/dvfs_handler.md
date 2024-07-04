\ingroup GroupModules Modules
\defgroup GroupDvfsHandler DvfsHandler

# Module DVFS Handler Architecture

Copyright (c) 2024, Arm Limited. All rights reserved.

# Overview

The DVFS (Dynamic Voltage and Frequency Scaling) Handler peripheral in the LCP
subsystem allows a software agent to abstract the details of programming the
DVFS settings for a component's clock frequency and voltage level. The interface
permits software to program a frequency and voltage level by writing to specific
registers, making the hardware responsible for managing the lower-level details
of applying these settings to the required components. This eliminates the need
for software to support SoC-specific and complex programming sequences, thereby
reducing the overall software load.

The DVFS handler module provides 'policy voltage' and 'policy frequency'
registers, which simplifies software's role in achieving frequency and voltage
scaling by allowing direct writes to these registers.

# Module design

The DVFS handler module is registered with the MHU fastchannel through the
transport HAL. This enables the application processor or any other element to
communicate with this module via the fastchannel and request performance levels
to be set. On module start, the DVFS handler module will register the set level
callback with the MHU module. On receiving a fastchannel message, the registered
callback is invoked and the request is accomplished.

## Performance level set

The non-secure OS running on the AP core will request a performance change via
the MHU fastchannel. The requested performance level from the OS is on an
absolute scale. Upon receiving the request, the module will check the OPP table
to determine if the requested performance level on the absolute scale is
available. If the level is present, the module will use the voltage and
frequency settings corresponding to that level. If the requested performance
level is not directly available, the module will use an entry that is present in
the OPP table, which is just below the requested performance level.

Here's an example of a DVFS OPP table:

```C
    static struct mod_dvfs_handler_opp dvfs_handler_opps[] = {
        { .level = 85UL, .frequency = 1700 * FWK_MHZ, .voltage = 850000 },
        { .level = 100UL, .frequency = 2000 * FWK_MHZ, .voltage = 900000 },
        { .level = 115UL, .frequency = 2300 * FWK_MHZ, .voltage = 950000 },
        { .level = 130UL, .frequency = 2600 * FWK_MHZ, .voltage = 950000 },
        { .level = 145UL, .frequency = 2900 * FWK_MHZ, .voltage = 1000000 },
        { .level = 160UL, .frequency = 3200 * FWK_MHZ, .voltage = 1050000 },
        { 0 }
    };
```

The DVFS handler hardware expects the frequency in megahertz (MHz) and voltage
in microvolts (uV). Upon finalizing the performance level, the module will
retrieve the frequency and voltage settings from the opps table and program
them into the 'policy frequency' and 'policy voltage' registers, respectively.
After updating the voltage and frequency, the module will check the 'policy
frequency status' and 'policy voltage status' registers to ensure that the
updated values are correctly reflected.

# Configuration Example

The following examples demonstrate how to populate the config data for the DVFS
handler driver module. An element corresponds to dvfs physical domain here.

```C
    static const struct fwk_element element_table[] = {
        [0] = {
            .name = "LCP-CPU0-Domain",
            .data = &((struct mod_dvfs_handler_config) {
                .sustained_idx = 4,
                .dvfs_handler_addr = LCP_DVFS_FRAME_BASE,
                .opps = dvfs_handler_opps,
                .dvfs_fch_set_level = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_MHU3,
                    MHU3_DEVICE_IDX_LCP_AP_FCH_DVFS_SET_LVL),
            }),
        },
        [1] = { 0 },
    };

    static const struct fwk_element *dvfs_handler_get_element_table(
        fwk_id_t module_id)
    {
        return element_table;
    }

    const struct fwk_module_config config_dvfs_handler = {
        .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dvfs_handler_get_element_table)
    };

```
