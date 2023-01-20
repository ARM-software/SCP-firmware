\ingroup GroupModules Modules
\defgroup GroupAtu Smcf

# System Monitoring Control Framework SMCF Architecture

Copyright (c) 2023, Arm Limited. All rights reserved.

## Overview

The System Monitoring Control Framework is designed to manage a large and
diverse set of on-chip sensors and monitors. It does this by presenting
software with a standard interface to control the monitors, regardless of
type, and reducing software load of controlling the monitor sampling and data
collection.

The SMCF reduces the burden on monitor control by enabling sampling on multiple
monitors to be controlled together and by various triggers either internal or
external to the SMCF. The number of monitors that the SMCF supports can be
configured. The SMCF eases data collection requirements by allowing the data
from multiple monitors to be collated in a single location or writing out data
to a memory-mapped location that is easier for the monitoring agent to access.

## Module Design

SMCF can manage sensors, activity counters or any continuously changing system
data. Each data source is called a monitor and connects to an MLI (Monitor Local
Interface). The data width of each monitor could be anything from one bit to 64
bits. Each group of MLI's is connected to one MGI (Monitor Group Interface),
which provides the software interface and a set of functions to be applied to a
group of monitors.
Each MGI is an element or a domain in the module implementation, while the
monitor (MLI) represents a sub-element. Module configuration is required for
elements only, i.e. MGI.


## Sample Type

SMCF provides four different types of sampling data.

1) **Manual Trigger:**
When software wants to perform a single sampling of the monitor group.

2) **Periodic Sample:**
Continuous sampling based on programmed period. For example, an IoT system
monitors the temperature periodically.

3) **Data Read:**
When the sample is required to be started when the data from the previous
monitor sample data set is consumed. A new sample begins when the last data
value from a monitor sample data set is read.

4) **Input Trigger:**
When a sample is required to be started from an event that is external to an
MGI.


## Module Interface
The module provides the following interfaces.

### Data related API:
This interface provides the functions required to start the data sampling and
get the data.

### Notifications:
An SMCF client module can register to listen for notifications when a new data
sample is available. The client should then use the data API to get the sampled
data. There is no guarantee that the sample will still be valid when the client
requests the data.

### Interrupt API:
This interface is used when another module handles the hardware IRQ. For example
another module captures the interrupt, performs some actions, and then calls
this interface to clear the interrupt and execute an internal handler.
Currently, only support for "SAMPLE COMPLETE" type of event is available.
This internal handler will initiate the sequence to notify listeners of a
completed sample (sample is ready). In such case, the IRQ line is not populated
in the SMCF configuration (FWK_INTERRUPT_NONE), and the respective interrupt is
enabled during the bind call.

### Control API:
The control interface provides a generic method for controlling monitors.
The specific detail of the functionality depends on the monitor itself.

## Module Configuration
The SMCF module requires configuration per element. Therefore, a table of
element configuration must be provided. However, there is no module-specific
configuration data.

### Element configuration entries:

| Configuration         | Description                             |
|-----------------------|-----------------------------------------|
| MGI register address  | Start address of the MGI registers      |
| MGI IRQ Number        | MGI IRQ if available                    |
| Sample Type           | The monitor sample type                 |
| DMA Address           | Optional DMA address                    |
| Operational Mode      | Monitor specific optional configuration |


### Configuration Example (One MGI, sample type input trigger)
```c
    static const struct fwk_element element_table[2] = {
        [0] = {
            .name = "MGI0",
            .data = &(struct mod_smcf_mgi_config mgi_config = {
                .reg_base = 0xFFFF0000,
                .irq = 31,
                .sample_type = SMCF_SAMPLE_TYPE_TRIGGER_INPUT,
                .data_config = {
                    .header_format = 0,
                    .data_location = SMCF_DATA_LOCATION_RAM,
                    .write_addr = (uint64_t)0x51000000,
                    .read_addr = (uint32_t *)0x51000000,
                },
            }
        },
        [1] = { 0 },
    };

    struct fwk_module_config config_smcf = {
        .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(element_table),
    };
```