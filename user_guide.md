# SCP-firmware User Guide

This document describes how to build the SCP and MCP firmware and run it with a
tested set of other software components using defined configurations on
supported Arm platforms. While it is possible to use other software components,
configurations and platforms, how to do so is outside the scope of this
document.

## Host machine requirements

Running SCP-firmware on Fixed Virtual Platform (FVP) models requires at least
12GB of available memory. A multicore CPU is highly recommended to maintain
smooth operation.

This software has been tested on Ubuntu 18.04 LTS (64-bit).

## Prerequisites

To build the SCP/MCP firmware for a target product, the following tools are
required:

- [GNU Make] (*4.2* or later)
- [Python 3] (*3.6.9* or later)

[GNU Make]: https://www.gnu.org/software/make/
[GNU Make]: https://www.python.org/downloads/release/python-369/

Additionally, the firmware may be built using one of two compilers:

- [GNU Arm Embedded Toolchain] (*9-2019-q4* or later)
- [Arm Compiler 6] (*6.13* or later)

[GNU Arm Embedded Toolchain]: https://developer.arm.com/open-source/gnu-toolchain/gnu-rm
[Arm Compiler 6]: https://developer.arm.com/tools-and-software/embedded/arm-compiler/downloads/version-6

For Juno, it is required to have a more recent of GNU Arm embedded toolchain.
We recommend to use at least the following release: 9-2019-q4-major.

The following tools are recommended but not required:

- [Doxygen] (*1.8.13* or later): Required to generate supporting documentation
- [GNU GCC] (*7.4.0* or later): Required to build framework tests that run on
    the host system

[Doxygen]: http://www.doxygen.nl/
[GNU GCC]: https://gcc.gnu.org/

If building for an Arm FVP platform, you will need to ensure you have [the
relevant FVP].

[the relevant FVP]: https://developer.arm.com/products/system-design/fixed-virtual-platforms

The FVPs also have a soft dependency on the following tools:

- [xterm]: Required to view UART output

[xterm]: https://invisible-island.net/xterm/

The instructions provided as a part of this guide assume you have [Git]
(*2.17.1* or later) available in your environment.

[Git]: https://git-scm.com/

Installing these prerequisites can be done on any standard Debian-based system
with the following:

```sh
sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
sudo apt update
sudo apt install build-essential doxygen gcc-arm-embedded git python3
```

For the FVP prerequisites:

```sh
sudo apt install xterm
```

## Cloning the SCP-firmware source code

The SCP-firmware source code can be cloned from the official GitHub repository:

```sh
git clone --recurse-submodules https://github.com/ARM-software/SCP-firmware.git ${SCP_PATH}
```

### Cloning dependencies

Under certain configurations the SCP-firmware has a dependency on the CMSIS-Core
and CMSIS-RTOS2 projects, which are part of the [Cortex Microcontroller System
Interface Standard (CMSIS)] software pack. The source tree for this software is
included with the firmware as a Git submodule. You can fetch all submodules from
within the source directory with the following:

[Cortex Microcontroller System Interface Standard (CMSIS)]: https://www.arm.com/why-arm/technologies/cmsis

```sh
git submodule update --init
```

## Documentation

If Doxygen is available on the host system then comprehensive documentation can
be generated. The complete set of documentation is compiled into bundles in
HTML, LaTeX, and XML formats and placed in the `build/doc` directory. This
documentation includes:

- A *README* section
- The BSD-3-Clause license under which this software and supporting files are
    distributed
- The SCP-firmware user guide (the content of this file)
- An overview of the framework on which SCP-firmware is written, including
    information on how components of the software must interact with the
    framework and with each other
- An overview of the build system and the project directory structure
- The project's coding style
- Source documentation for the SCP-firmware framework
- Source documentation for modules that are included in the currently supported
    products

From within the SCP-firmware root directory Doxygen can be invoked using the
top-level Makefile:

```sh
make doc
```

## Building SCP-firmware

To build SCP-firmware for a specific product the basic command format for
invoking `make` (from within the source directory) is:

```sh
make CC=<COMPILER> PRODUCT=<PRODUCT> [OPTIONS] [TARGET]
```

For example, to build the RAM firmware for SGM-775 in debug mode, use the
following:

```sh
make CC=arm-none-eabi-gcc PRODUCT=sgm775 MODE=debug firmware-scp_ramfw
```

The `all` target will be used if `[TARGET]` is omitted, which will build all the
firmware defined by the product.

The `help` target provides further information on the arguments that can be
given:

```sh
make help
```

The framework includes a suite of tests that validate its core functionality.
If you installed the native GCC prerequisite, these can be run on the host
system using:

```sh
make test
```

For all products other than `host`, the code needs to be compiled by a
cross-compiler. The toolchain is derived from the `CC` variable, which should
point to the cross-compiler executable. It can be set as an environment variable
before invoking `make`, or provided as part of the build command:

```sh
make CC=<path to the cross compiler> ...
```

For more guidance and information on the build system, refer to the full set of
documentation included in the *Build System* chapter of the Doxygen-generated
documentation.

## Running the SCP firmware on SGM platforms

For an introduction to the System Guidance for Mobile (SGM) platforms, please
refer to [the Arm Developer documentation].

[the Arm Developer documentation]: https://community.arm.com/developer/tools-software/oss-platforms/w/docs/388/system-guidance-for-mobile-sgm

The instructions within this section use SGM-775 as an example platform, but
they are relevant for all SGM platforms.

### Building the images

The build system generates firmware images per the `product.mk` file associated
with the product. For SGM platforms, two firmware images are built:

- `scp_romfw.bin`: SCP ROM firmware image - handles the transfer of the RAM
    firmware to private SRAM and jumps to it
- `scp_ramfw.bin`: SCP RAM firmware image - manages the system runtime services

```sh
cd ${SCP_PATH} && \
    make CC=arm-none-eabi-gcc PRODUCT=sgm775 MODE=debug

export SCP_ROM_PATH=${SCP_PATH}/build/product/sgm775/scp_romfw/debug/bin/scp_romfw.bin
export SCP_RAM_PATH=${SCP_PATH}/build/product/sgm775/scp_ramfw/debug/bin/scp_ramfw.bin
```

### Booting the firmware

In order for the `scp_ramfw.bin` firmware image to be loaded, an application
processor secure world firmware needs to be available to load it. Arm maintains
the [Arm Trusted Firmware-A (TF-A)] project, which handles this case. The
remaining instructions assume you are using Trusted Firmware-A.

[Arm Trusted Firmware-A (TF-A)]: https://github.com/ARM-software/arm-trusted-firmware

On SGM platforms, the SCP images are given alternative names when used in the
context of TF-A:

- `scp_romfw.bin` has the alternative name `scp_bl1`
- `scp_ramfw.bin` has the alternative name `scp_bl2`

To boot the SCP firmware on SGM platforms with TF-A, you will need at minimum
three additional images:

- `bl1`: BL1 - first-stage bootloader stored in the system ROM
- `bl2`: BL2 - second-stage bootloader loaded by `bl1`, responsible for handing
    over `scp_bl2` to the SCP
- `fip`: FIP - firmware image package containing `bl2` and `scp_bl2`

The FIP format acts as a container for a number of commonly-used images in the
TF-A boot flow. Documentation for the FIP format can be found in the [TF-A
firmware design documentation].

[TF-A firmware design documentation]: https://github.com/ARM-software/arm-trusted-firmware/blob/cfb3f73344217aa000aaff9d84baad7527af75bf/docs/design/firmware-design.rst#firmware-image-package-fip

An example command line to build Arm Trusted Firmware-A for AArch64 is given
below. Note that you will need to have installed [the prerequisites for building
Arm Trusted Firmware-A for SGM-775].

[the prerequisites for building Arm Trusted Firmware-A for SGM-775]: https://github.com/ARM-software/arm-trusted-firmware/blob/v2.1/docs/user-guide.rst#tools

```sh
export TFA_PATH=<your Trusted Firmware-A path>

git clone -b v2.1 https://github.com/ARM-software/arm-trusted-firmware.git ${TFA_PATH}

cd ${TFA_PATH}

make CROSS_COMPILE=aarch64-linux-gnu- DEBUG=1 LOG_LEVEL=30 PLAT=sgm775 CSS_USE_SCMI_SDS_DRIVER=1 \
    bl1 bl2 fiptool

export BL1_PATH=${TFA_PATH}/build/sgm775/debug/bl1.bin
export BL2_PATH=${TFA_PATH}/build/sgm775/debug/bl2.bin
export FIP_PATH=/tmp/fip.bin

./tools/fiptool/fiptool create \
    --tb-fw ${BL2_PATH} \
    --scp-fw ${SCP_RAM_PATH} \
        ${FIP_PATH}
```

Note that `CSS_USE_SCMI_SDS_DRIVER` is a work-around for the fact that the v2.1
utilises **SCPI** instead of **SCMI** by default, which is not a supported
configuration for SCP-firmware.

To simulate the basic SCP boot flow on the SGM-775 FVP, use the following
command line:

```sh
FVP_CSS_SGM-775 \
    -C css.trustedBootROMloader.fname=${BL1_PATH} \
    -C css.scp.ROMloader.fname=${SCP_ROM_PATH} \
    -C board.flashloader0.fname=${FIP_PATH}
```

Note that it's expected that TF-A will crash, as we have not provided the full
bootloader image chain.

## Running the SCP firmware on SGI and Neoverse Reference Design platforms

For an introduction to the System Guidance for Infrastructure (SGI) platforms,
please refer to [System Guidance for Infrastructure (SGI)].

[System Guidance for Infrastructure (SGI)]: https://community.arm.com/developer/tools-software/oss-platforms/w/docs/387/system-guidance-for-infrastructure-sgi

For an introduction to the Neoverse Reference Design (RD) platforms, please
refer to [Neoverse Reference Designs].

[Neoverse Reference Designs]: https://community.arm.com/developer/tools-software/oss-platforms/w/docs/387/system-guidance-for-infrastructure-sgi

The instructions within this section use SGI-575 as an example platform, but
they are relevant for all SGI and Neoverse Reference Design platforms.

### Building the images

The build system generates firmware images per the `product.mk` file associated
with the product. For SGI and Neoverse Reference Design platforms, three
firmware images are built:

- `scp_romfw.bin`: SCP ROM firmware image - loads the SCP RAM firmware from NOR
    flash into private SRAM and jumps to it
- `scp_ramfw.bin`: SCP RAM firmware image - manages the system runtime services
- `mcp_romfw.bin`: MCP ROM firmware image

```sh
cd ${SCP_PATH} && \
    make CC=arm-none-eabi-gcc PRODUCT=sgi575 MODE=debug

export SCP_ROM_PATH=${SCP_PATH}/build/product/sgi575/scp_romfw/debug/bin/scp_romfw.bin
export SCP_RAM_PATH=${SCP_PATH}/build/product/sgi575/scp_ramfw/debug/bin/scp_ramfw.bin
export MCP_ROM_PATH=${SCP_PATH}/build/product/sgi575/mcp_romfw/debug/bin/mcp_romfw.bin
```

### Creating the NOR flash image

Unlike in the System Guidance for Mobile platforms, a secure-world application
processor firmware is not required to load the SCP firmware. Instead, the SCP
ROM firmware loads the SCP RAM firmware directly from NOR flash memory at a
fixed address.

To create a NOR flash image containing only the SCP RAM firmware, use:

```sh
export NOR_PATH=/tmp/nor.bin

dd if=/dev/zero of=${NOR_PATH} bs=1024 count=62976 && \
cat ${SCP_RAM_PATH} >> ${NOR_PATH}
```

### Booting the firmware

To simulate the basic SCP boot flow on the SGI-575 FVP, use the following
command line:

```sh
FVP_CSS_SGI-575 \
    -C css.scp.ROMloader.fname=${SCP_ROM_PATH} \
    -C css.mcp.ROMloader.fname=${MCP_ROM_PATH} \
    -C board.flashloader0.fname=${NOR_PATH}
```

## Running the SCP firmware on the Juno Development Board

For an introduction to the Juno Development Board, please refer to
[the Arm Developer documentation].

The instructions within this section are similar to those used for SGM
platforms, with minor differences.

[the Arm Developer documentation]: https://developer.arm.com/tools-and-software/development-boards/juno-development-board

### Building the images

Like for SGM platforms, the build system generates two images. For Juno, an
additional binary is generated:

- `scp_romfw_bypass.bin`: SCP ROM bypass firmware image - an alternative ROM
    firmware that is loaded from an external non volatile on-board memory.
    This binary needs to be used in order to successfully load the SCP RAM
    firmware, and is chain-loaded from the burned-in ROM on the physical board
    (not necessary for the FVP).

We recommend using the latest release of Trusted Firmware-A (version 2.2 as of
writing). Please note that using a debug version of the firmware is currently
only supported on the `integration` branch of Trusted Firmware-A, as additional
fixes are required to support the larger image size.

### Booting the firmware

The same steps for creating the FIP binary described by the SGM platforms
sections can be applied here. When invoking `make` for TF-A, make sure you
replace `PLAT=sgm775` with `PLAT=juno`.

Before proceeding with the boot on Juno, make sure you have all of the following
binaries:

- `scp_romfw_bypass.bin`
- `fip.bin`
- `bl1.bin`

Before beginning, please ensure the SD card used for your Juno board has been
set up with a Linaro release software stack. If this is not the case, you can
follow the [Linaro software release instructions] and/or download a new SD card
filesystem from the [Linaro releases page].

[Linaro software release instructions]: https://git.linaro.org/landing-teams/working/arm/arm-reference-platforms.git/about/docs/juno/user-guide.rst
[Linaro releases page]: https://releases.linaro.org/members/arm/platforms/

Once your SD card has been set up, you can do the following to get started with
building and running the SCP firmware:

- Replace `SOFTWARE/fip.bin` with your version of `fip.bin`
- Replace `SOFTWARE/bl1.bin` with your version of `bl1.bin`
- Replace `SOFTWARE/scp_bl1.bin` with your version of `scp_romfw_bypass.bin`

Lastly, ensure your host has synchronized any buffered data on the SD card (on
Linux and Unix systems, this can be done with the `sync` command) and reset the
board.

You can see the progress of the boot by connecting the UART to your host PC
(please follow the instructions in the Juno [Getting Started Guide]).

[Getting Started Guide]: https://static.docs.arm.com/den0928/f/DEN0928F_juno_arm_development_platform_gsg.pdf

## Software stack

Arm provides [a super-project] with guides for building and running a full
software stack on Arm platforms. This project provides a convenient wrapper
around the various build systems involved in the software stack, including for
SCP-firmware.

[a super-project]: https://git.linaro.org/landing-teams/working/arm/arm-reference-platforms.git/about/docs/user-guide.rst
