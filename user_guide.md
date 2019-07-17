SCP-firmware User Guide
=======================

This document describes how to build the SCP and MCP firmware and run it with a
tested set of other software components using defined configurations on
supported Arm platforms. While it is possible to use other software components,
configurations and platforms, how to do so is outside the scope of this
document.

Host machine requirements
-------------------------

Running SCP-firmware on Fixed Virtual Platform (FVP) models requires at least
12GB of available memory. A multicore CPU is highly recommended to maintain
smooth operation.

The software has been tested on Ubuntu 16.04 LTS (64-bit). Packages used for
building the software were installed from that distribution's official
repositories, unless otherwise specified.

Prerequisites (tools)
---------------------

To build the SCP/MCP firmware for a target product, the following tools are
required:

- Git
- [GNU Arm Embedded Toolchain ("6-2017-q2-update" or later)](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm)
- GNU Make (4.2 or later)
- Python 3 (3.5.0 or later)

In addition, the following tools are recommended:

- Doxygen (1.8.0 or later): Required to build supporting documentation
- GCC (6.0 or later): Required to build framework tests that run on the host
    system

Getting the SCP-firmware source code
------------------------------------

Download the SCP-firmware source code from Github:

    $> git clone https://github.com/ARM-software/SCP-firmware.git

Prerequisites (CMSIS Libraries)
-------------------------------

SCP-firmware uses components from the Cortex Microcontroller System Interface
Standard (CMSIS) software pack, specifically the CMSIS Core and CMSIS Real-Time
Operating System (RTOS) components. The CMSIS software pack is included as a Git
submodule.

To retrieve the required version of the CMSIS software, first ensure you have an
available Git installation, then from within the root directory of SCP-firmware
initialize and update the submodule with:

    $> git submodule update --init

Documentation
-------------

If Doxygen is available on the host system then comprehensive documentation can
be generated. The complete set of documentation is compiled into bundles in
HTML, LaTeX, and XML formats and placed in the *build/doc* directory. This
documentation includes:

- A *Readme* section
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

    $> make doc

Building SCP-firmware
---------------------

To build SCP-firmware for a specific product the basic command format for
invoking 'make' is:

    $> make <PRODUCT=<name>> [OPTIONS] [TARGET]

It is not necessary to provide a firmware target as the default target for the
product will build all the firmware contained within it.

The 'help' target provides further information on the arguments that can be
given:

    $> make help

The framework includes a suite of tests that validate its core functionality.
These can be run on the host system using:

    $> make test

For all products other than 'host', the code needs to be compiled by a
cross-compiler. The toolchain is derived from the CC parameter, which should
point to the cross-compiler. It can be set as an environment variable before
invoking 'make', or provided as part of the build command:

    $> make CC=<path to the cross compiler> ...

For more guidance and information on the build system, refer to the full set of
documentation included in the 'Build System' chapter of the documentation.

Running the SCP firmware on System Guidance for Mobile (SGM) platforms
----------------------------------------------------------------------

For an introduction to the System Guidance for Mobile (SGM) platforms, please
refer to [System Guidance for Mobile (SGM)](https://community.arm.com/dev-platforms/w/docs/388/system-guidance-for-mobile-sgm).

The build system generates the list of firmware images as defined by the
product.mk file associated with the product. For SGM platforms, two firmware
images are built: scp_romfw.bin and scp_ramfw.bin.

The scp_romfw.bin firmware image exists in the ROM of the system. It is the
first firmware image executed when booting an SGM platform. It does the minimal
setup and initialization of the system, and powers on the primary application
core. Then, it waits for the primary core to load the scp_ramfw.bin image into
secure RAM before handing over execution to it.

The scp_ramfw.bin firmware image provides all the runtime services delegated to
the SCP as a part of the system. It is loaded by the primary core and not the
scp_romfw.bin firmware to leverage the processing power of the application
processor for image authentication.

In order for the scp_ramfw.bin firmware image to be loaded per the boot flow
described above, a minimal application processor firmware needs to be available.
This typically consists of at least two images:

- bl1: First-stage bootloader stored in the system ROM
- bl2: Second-stage bootloader loaded by bl1, responsible for loading the
    scp_ramfw.bin firmware image and other application processor firmware images
    into system RAM

For more information about application processor firmware images and how to
build them, please refer to the [Arm Trusted Firmware-A user guide](https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/getting_started/user-guide.rst).

In order for the bl2 firmware image and the scp_ramfw.bin firmware image to be
made available to their respective loaders, they must be packaged in a Firmware
Image Package (FIP). Please refer to the Arm Trusted Firmware-A user guide for
instructions on building FIP packages.

To run the boot flow described above on an SGM platform FVP, use:

    $> <path to the SGM platform FVP> \
        -C soc.pl011_uart0.out_file=./ap.txt \
        -C soc.pl011_uart1.out_file=./scp.txt \
        -C css.scp.ROMloader.fname=<path to scp_romfw.bin firmware image> \
        -C css.trustedBootROMloader.fname=<path to bl1 firmware image> \
        -C board.flashloader0.fname=<path to FIP> \
        -C soc.pl011_uart1.unbuffered_output=1 \
        -C soc.pl011_uart0.unbuffered_output=1

Note:
    - SGM platform FVPs are available on
        [the Fixed Virtual Platforms product page](https://developer.arm.com/products/system-design/fixed-virtual-platforms).
    - The application processor firmware images can be built using the
        [Arm Platforms deliverables](https://community.arm.com/dev-platforms/w/docs/304/arm-platforms-deliverables).
        See the following section.

Running the SCP/MCP firmware on System Guidance for Infrastructure platforms
----------------------------------------------------------------------------

For an introduction to the System Guidance for Infrastructure (SGI) platforms,
please refer to [System Guidance for Infrastructure (SGI)](https://community.arm.com/developer/tools-software/oss-platforms/w/docs/387/system-guidance-for-infrastructure-sgi).

The build system generates the list of firmware images as defined by the
product.mk file associated with the product. For SGI platforms, three firmware
images are built: scp_romfw.bin, mcp_romfw.bin and scp_ramfw.bin.

The scp_romfw.bin and mcp_romfw.bin firmware images exist in the ROM of the
system. They are the first firmware images executed when booting an SGI
platform.

The scp_romfw.bin image does the minimal setup and initialization of the system
and then loads from the flash the scp_ramfw.bin image into secure RAM before
handing over execution to it.

The scp_ramfw.bin firmware image provides all the runtime services delegated to
the SCP as a part of the system.

The mcp_romfw.bin image initializes the MCP.

To run the boot flow described above on an SGI platform FVP, use:

    $> <path to the SGI platform FVP> \
        -C css.scp.pl011_uart_scp.out_file=./scp.txt \
        -C css.mcp.pl011_uart0_mcp.out_file=./mcp.txt \
        -C css.scp.ROMloader.fname=<path to scp_romfw.bin firmware image> \
        -C board.flashloader0.fname=<path to nor.bin file (see below)> \
        -C css.mcp.ROMloader.fname=<path to mcp_romfw.bin firmware image> \
        -C css.mcp.pl011_uart0_mcp.unbuffered_output=1 \
        -C css.scp.pl011_uart_scp.unbuffered_output=1

Note:
    - SGI platform FVPs are available on
        [the Fixed Virtual Platforms product page](https://developer.arm.com/products/system-design/fixed-virtual-platforms).
    - The scp_romfw.bin image expects the scp_ramfw.bin image to be located at
      the offset 0x03D80000 in the NOR flash. The content of the NOR flash is
      passed to the model as a nor.bin binary file. The nor.bin file can be
      build in the current working directory as follow:

      $> dd if=/dev/zero of=nor.bin bs=1024 count=62976
      $> cat <path to scp_ramfw.bin image> >> nor.bin

      The first command creates a nor.bin file of size 0x3D80000 filed in with
      zeroes. The second command appends the scp_ramfw.bin image to the nor.bin
      file.

Running the SCP/MCP firmware on Neoverse reference designs
----------------------------------------------------------

For an introduction to the Neoverse reference designs,
please refer to [Neoverse reference designs](https://community.arm.com/developer/tools-software/oss-platforms/w/docs/443/neoverse-reference-designs).

The build system generates the list of firmware images as defined by the
product.mk file associated with the product. For Neoverse reference designs,
three firmware images are built: scp_romfw.bin, mcp_romfw.bin and
scp_ramfw.bin.

The scp_romfw.bin and mcp_romfw.bin firmware image exist in the ROM of the
system. They are the first firmware images executed when booting a Neoverse
reference design.

The scp_romfw.bin image does the minimal setup and initialization of the system
and then loads the scp_ramfw.bin image into secure RAM before handing over
execution to it.

The scp_ramfw.bin firmware image provides all the runtime services delegated to
the SCP as a part of the system.

The mcp_romfw.bin image initializes the MCP.

To run the boot flow described above on an Neoverse reference design FVP, use:

    $> <path to the Neoverse reference design FVP> \
        -C css.scp.pl011_uart_scp.out_file=./scp.txt \
        -C css.mcp.pl011_uart0_mcp.out_file=./mcp.txt \
        -C css.scp.ROMloader.fname=<path to scp_romfw.bin firmware image> \
        -C board.flashloader0.fname=<path to nor.bin file (see below)> \
        -C css.mcp.ROMloader.fname=<path to mcp_romfw.bin firmware image> \
        -C css.mcp.pl011_uart0_mcp.unbuffered_output=1 \
        -C css.scp.pl011_uart_scp.unbuffered_output=1

Note:
    - Neoverse reference designs FVPs are available on
        [the Fixed Virtual Platforms product page](https://developer.arm.com/products/system-design/fixed-virtual-platforms).

    - The scp_romfw.bin image expects the scp_ramfw.bin image to be located at
      the offset 0x03D80000 in the NOR flash. The content of the NOR flash is
      passed to the model as a nor.bin binary file. The nor.bin file can be
      build in the current working directory as follow:

      $> dd if=/dev/zero of=nor.bin bs=1024 count=62976
      $> cat <path to scp_ramfw.bin image> >> nor.bin

      The first command creates a nor.bin file of size 0x3D80000 filed in with
      zeroes. The second command appends the scp_ramfw.bin image to the nor.bin
      file.

Booting up to the Linux prompt on Arm platforms
-----------------------------------------------

The [Arm Reference Platforms deliverables](https://community.arm.com/dev-platforms/w/docs/304/arm-reference-platforms-deliverables)
provide a set of source code bases and prebuilt binaries of a fully bootable
Linux software stack on supported Arm platforms.

This section explains how to update the SCP-firmware binaries once the full
software stack has been fully built from source for a given configuration.

To retrieve, build and run the software stack from source for a given Arm
platform, please refer to [Run the Arm Platforms deliverables on an FVP](https://community.arm.com/dev-platforms/w/docs/392/run-the-arm-platforms-deliverables-on-an-fvp).

Note that the script initializing the workspace does not currently download the
gcc-arm-none-eabi-5_4-2016q3 toolchain needed to build SCP-firmware. As
such, you will need to download it from [here](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)
and extract it into the workspace/tools/gcc/gcc-arm-none-eabi-5_4-2016q3
directory.

Once the software stack has been retrieved, the SCP/MCP source code can be found
in the 'scp' directory at the root of the workspace.

To rebuild the SCP-firmware binaries without rebuilding the whole software
stack, at the root of the framework, use:

    $> ./build-scripts/build-scp.sh build

Once the software stack has been fully built, use the following to update the
SCP binaries in the software package to be run:

    $> ./build-scripts/build-all.sh package

As an example, to run the software stack, on the SGM-775 FVP:

    $> export MODEL=/path/to/where/you/separately/installed/FVP_CSS_SGM-775
    $> cd ./model-scripts/sgm775
    $> ./run_model.sh -t sgm775

The SCP ROM/RAM firmware logs are written to the FVP terminal_s1 window, where
the firmware tag or commit identifier can also be found.
