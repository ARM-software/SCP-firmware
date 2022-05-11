# SynQuacer Platform Options

## Overview

This documentation describes what options are available and
how to build example on SynQuacer platform.

## Selecting an UART Port for SCP-firmware console

At SCP-firmware build time for SynQuacer platform,
the following 3 UART ports are selectable.

    uart0 : A standard UART on the 40 pin expansion connector (used as default)
    uart1 : A second standard UART on the 40 pin expansion connector
    debug_uart : A debug UART via the micro-B USB connector

To select UART port,
set SYNQUACER_UART value as below when executing make commnad.

If SYNQUACER_UART value is not set,
uart0 is selected as defalut.

In Make build system,

```sh
make
    CC=$CC \
    PRODUCT=synquacer \
    MODE=<debug,release> \
    [SYNQUACER_UART=<uart0,uart1,debug_uart>]
```

e.g.
make CC=$CC PRODUCT=synquacer MODE=release SYNQUACER_UART=uart1

In CMake build system,

```sh
make -f Makefile.cmake \
    CC=$CC \
    PRODUCT=synquacer \
    MODE=<debug,release> \
    [EXTRA_CONFIG_ARGS="-DSYNQUACER_UART=<uart0,uart1,debug_uart>]
```

e.g.
make -f Makefile.cmake CC=$CC PRODUCT=synquacer MODE=release EXTRA_CONFIG_ARGS="-DSYNQUACER_UART=uart1"

