\ingroup GroupModules Modules
\defgroup GroupAtu Atu

# Module ATU Architecture

Copyright (c) 2023, Arm Limited. All rights reserved.

# Overview

The Address Translation Unit (ATU) provides means to convert the logical address
(32 bit LA) from the MSCP memory space to the physical address residing in the
system memory.

This module implements the driver for configuring the Address Translation Unit.

# Module Design

This module provides an interface for other modules to configure address
translation regions with the output bus attributes during the runtime phase.

The module's functionalities depend on the following build flags:
- `SCP_ENABLE_ATU_MANAGE`
- `SCP_ENABLE_ATU_DELEGATE`

Depending on the requirement, the module can be used in the following ways:

1) `SCP_ENABLE_ATU_DELEGATE` set to TRUE:

    The module does not have access to ATU configuration registers. A component
    that resides outside the MSCP subsystem, which is the Root of Trust,
    configures the MSCP ATU. During the runtime, if ATU reconfiguration
    is required, then the ATU driver can delegate the requests to the RoT.

2) `SCP_ENABLE_ATU_MANAGE` set to TRUE:

    The module/SCP has access to the ATU configuration registers. The ATU driver
    configures the list of ATU regions from the module config data and during
    runtime, ATU can be reconfigured by the ATU driver directly provided that
    the MSCP has permissions to configure the ATU.

Note: Both the `SCP_ENABLE_ATU_DELEGATE` and the `SCP_ENABLE_ATU_MANAGE` flags
can be set to TRUE if the MSCP has multiple ATU devices and one or more ATU
devices need to be managed directly by the MSCP firmware.

## ATU configuration requests

When `SCP_ENABLE_ATU_DELEGATE` flag is set to TRUE, the ATU driver module binds
to the transport channel specified in the element config data. During the
runtime phase, this transport channel is used to send ATU configuration requests
and receive responses.

The module also binds to the Timer API, which is used to wait for a response.
If no response is received before the timeout, then the request is considered to
be failed.

As soon as the response is received, the transport module signals the ATU driver
module. The ATU driver sets a flag to indicate that a response has been received
and copies the message header and the payload into the `recv_msg_buffer`
structure which is used to store the most recently received response. The driver
then verifies the response and returns the status to the caller.

## ATU Region Output Bus Attributes

The following output bus attributes can be configured for each ATU region:

1) AxPROT0:
Affects the AxPROT[0] signal (Privileged access) at the output bus.

2) AxPROT1:
Affects the AxPROT[1] signal (Non-secure access) at the output bus.

3) AxPROT2:
Affects the AxPROT[2] signal (Instruction access) at the output bus.

4) AxCACHE0:
Affects the AxCACHE[0] signal (Bufferable) at the output bus.

5) AxCACHE1:
Affects the AxCACHE[1] signal (Modifiable) at the output bus.

6) AxCACHE2:
Affects the AxCACHE[2] signal (Allocate) at the output bus.

7) AxCACHE3:
Affects the AxCACHE[3] signal (Other Allocate) at the output bus.

8) AxNSE:
Affects the AxNSE signal at the main AXI Interconnect.

The supported values for each of the output bus attribute can found in the
[Arm ATU Architecture specification](https://developer.arm.com/documentation/107714/0000/Programmers-model/ATU-register-summary/ATUROBA-n---Region-Output-Bus-Attributes-n-register?lang=en)

The ATU driver module provides the following macros to encode the output bus
attributes into a 32 bit variable which can be used when invoking the ATU API or
in the ATU driver config data.

- `ATU_ENCODE_ATTRIBUTES(AXNSE_VAL, AXCACHE3_VAL, AXCACHE2_VAL, AXCACHE1_VAL,
   AXCACHE0_VAL, AXPROT2_VAL, AXPROT1_VAL, AXPROT0_VAL)`

    This macro can be used to be configure each output bus attribute to a
    specific value.

- `ATU_ENCODE_ATTRIBUTES_ROOT_PAS`

    This macro can be used to configure the ATU region for Root access. Only
    AxNSE and AxPROT1 values are modified. Other attributes are set to default
    values.

- `ATU_ENCODE_ATTRIBUTES_SECURE_PAS`

    This macro can be used to configure the ATU region for secure access. Only
    AxNSE and AxPROT1 values are modified. Other attributes are set to default
    values.

- `ATU_ENCODE_ATTRIBUTES_NON_SECURE_PAS`

    This macro can be used to configure the ATU region for non-secure access.
    Only AxNSE and AxPROT1 values are modified. Other attributes are set to
    default values.

- `ATU_ENCODE_ATTRIBUTES_REALM_PAS`

    This macro can be used to configure the ATU region for realm access. Only
    AxNSE and AxPROT1 values are modified. Other attributes are set to default
    values.

# ATU API Usage

The ATU API provides functions to add or remove translation regions in the
translation window. The API usage remains the same irrespective of whether the
MSCP configures the ATU directly or delegates the request to a different
firmware.

The following example demonstrates how a module can utilize the ATU API:

- Bind to the `MOD_ATU_API_IDX_ATU` API during the bind stage.

```C
    static const struct mod_atu_api *atu_api;

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_ATU),
        FWK_ID_API(FWK_MODULE_IDX_ATU, MOD_ATU_API_IDX_ATU),
        &atu_api);
```

- Use the API to add or remove translation regions.

```C
    /*
     * Variable to hold the region index returned by the the add_region()
     * function in the ATU interface after successful mapping.
     */
    uint8_t region_idx;

    /* Identifier of the ATU device */
    fwk_id_t atu_device_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_ATU, 0);

    /* Identifier of the entity that owns the ATU region */
    fwk_id_t atu_region_owner_id =
    FWK_ID_MODULE_ID_INIT(FWK_MODULE_IDX_CMN_CYPRUS);

    struct atu_region_map region = {
      /* Identifier of the entity that owns the ATU region */
      .region_owner_id = atu_region_owner_id,
      /* Logical address start */
      .log_addr_base = 0x78400000,
      /* Physical address start */
      .phy_addr_base = 0x2A4A0000,
      /* Size of the region */
      .region_size = (16 * FWK_KIB),
      /* Output bus attributes */
      .attributes = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
    };

    status = atu_api->add_region(&region, atu_device_id, &region_idx);

    if (status != FWK_SUCCESS) {
        return FWK_E_PANIC;
    }

    /* Do some work */

    /*
     * Optionally, remove the address translation region, when it's no longer
     * needed.
     * Note: Only region owner can remove the corresponding ATU region.
     */
     status = atu_api->remove_region(region_idx, atu_device_id,
         atu_region_owner_id);

    if (status != FWK_SUCCESS) {
        return FWK_E_PANIC;
    }

```

# Configuration Example

The following examples demonstrate how to populate the config data for the ATU
driver module.

## MSCP has ATU access

In this case, the MSCP manages the ATU directly and the ATU driver module config
data consists of the required translation regions to be configured at boot-time.

```config_atu.c```

```C
const struct atu_region_map atu_regions[ATU_REGION_IDX_COUNT] = {
    [ATU_REGION_IDX_CMN] = {
        .region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_PLATFORM_SYSTEM),
        .log_addr_base = 0xA0000000,
        .phy_addr_base = 0x100000000,
        .region_size = (1 * FWK_GIB),
        .attributes = ATU_ENCODE_ATTRIBUTES_ROOT_PAS,
    },
    [ATU_REGION_IDX_CLUSTER_UTIL] = {
        .region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_PLATFORM_SYSTEM),
        .log_addr_base = 0x60000000,
        .phy_addr_base = 0x200000000,
        .region_size = (256 * FWK_MIB),
        .attributes = ATU_ENCODE_ATTRIBUTES_ROOT_PAS,
    },
    [ATU_REGION_IDX_SHARED_SRAM] = {
        .region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_PLATFORM_SYSTEM),
        .log_addr_base = 0x70000000,
        .phy_addr_base = 0x00000000,
        .region_size = (2 * FWK_MIB),
        .attributes = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
    },
    [ATU_REGION_IDX_AP_PERIPH] = {
        .region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_PLATFORM_SYSTEM),
        .log_addr_base = 0x90000000,
        .phy_addr_base = 0x20000000,
        .region_size = (256 * FWK_MIB),
        .attributes = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
    },
};

static const struct fwk_element element_table[2] = {
    [0] = {
        .name = "SCP_ATU",
        .data = &(struct mod_atu_device_config) {
            .is_atu_delegated = false,
            .atu_base = SCP_ATU_BASE,
            .atu_region_config_table = atu_regions,
            .atu_region_count = FWK_ARRAY_SIZE(atu_regions),
        },
    },
    [1] = { 0 },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_atu = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};
```

## MSCP does not have ATU access

In this scenario, the MSCP firmware does not have access to the ATU. The ATU
driver delegates the requests to another component during the runtime. The
config data provides the details of the transport channel to bind to,
in order to send and receive messages.

```config_atu.c```

```C
static const struct fwk_element element_table[] = {
    [0] = {
        .name = "SCP_ATU",
        .data = &(struct mod_atu_device_config) {
            .is_atu_delegated = true,
            .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
            .transport_id =
                FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_TRANSPORT,
                    SCP_PLATFORM_TRANSPORT_IDX_ATU),
        },
    },
    [1] = { 0 },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_atu = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};
```
