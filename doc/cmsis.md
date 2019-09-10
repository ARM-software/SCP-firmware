CMSIS Support and Integration
=============================

Overview                                                       {#cmsis_overview}
========

Firmware using components with a dependency on the CMSIS library will need to
configure CMSIS for the target device. This is done by way of a `<fmw_cmsis.h>`
header, which should provide the definitions necessary to ensure CMSIS is
properly configured for the device. It is recommended that this header resides
in the directory of the firmware.

While the build system does not enforce this strategy, for consistency it is
recommended that new architectures, products and firmwares follow it.

Example                                                         {#cmsis_example}
=======

The example below configures CMSIS to run on an Arm Cortex-M3 r2p1 with an MPU
and basic interrupts. The definitions used in this example are expected by
CMSIS, and are described in the CMSIS 5 documentation.

    #ifndef FMW_CMSIS_H
    #define FMW_CMSIS_H

    #define __CHECK_DEVICE_DEFINES
    #define __CM3_REV 0x0201
    #define __MPU_PRESENT 1
    #define __NVIC_PRIO_BITS 3
    #define __Vendor_SysTickConfig 0

    typedef enum IRQn {
        NonMaskableInt_IRQn   = -14,
        MemoryManagement_IRQn = -12,
        BusFault_IRQn         = -11,
        UsageFault_IRQn       = -10,
        SVCall_IRQn           = -5,
        DebugMonitor_IRQn     = -4,
        PendSV_IRQn           = -2,
        SysTick_IRQn          = -1,
    } IRQn_Type;

    #include <core_cm3.h>

    #endif /* FMW_CMSIS_H */

Sharing Configurations                                   {#cmsis_shared_configs}
======================

In some cases, it may be desirable to use the same CMSIS configuration for more
than one firmware in a product. The build system provides no provisions for
sharing a configuration, but the suggested strategy is laid out below:

    <root>
     └─ product
        └── <product>
            ├── include
            │   ├── fmw_cmsis_a.h
            │   ├── fmw_cmsis_b.h
            │   └── ...
            ├── a1
            │   ├── fmw_cmsis.h (includes <fmw_cmsis_a.h>)
            │   └── ...
            ├── a2
            │   ├── fmw_cmsis.h (includes <fmw_cmsis_a.h>)
            │   └── ...
            └── b
                ├── fmw_cmsis.h (includes <fmw_cmsis_b.h>)
                └── ...

Alternatively, for products with only one CMSIS configuration, it is reasonable
to move `<fmw_cmsis.h>` to the product include directory:

    <root>
     └─ product
        └── <product>
            ├── include
            │   ├── fmw_cmsis.h
            │   └── ...
            ├── a
            │   └── ...
            ├── b
            │   └── ...
            └── c
                └── ...
