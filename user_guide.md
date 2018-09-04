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

For all products other than 'host', the code needs to be compiled by a cross-
compiler. The toolchain is derived from the CC parameter, which should point to
the cross-compiler. It can be set as an environment variable before invoking
'make', or provided as part of the build command:

    $> make CC=<path to the cross compiler> ...

For more guidance and information on the build system, refer to the full set of
documentation included in the 'Build System' chapter of the documentation.

Running the SCP firmware on System Guidance for Mobile (SGM) platforms
----------------------------------------------------------------------

The build system generates the list of firmware images as defined by the
product.mk file associated with the product. For SGM platforms, two firmware
images are built, referred to as 'scp_romfw' and 'scp_ramfw' below.

The scp_romfw firmware image exists in the ROM of the system. It is the first
firmware image executed when booting an SGM platform. It does the minimal setup
and initialization of the system, and powers on the primary application core.
Then, it waits for the primary core to load the scp_ramfw image into secure RAM
before handing over execution to it.

The scp_ramfw firmware image provides all the runtime services delegated to the
SCP as a part of the system. It is loaded by the primary core and not the
scp_romfw firmware to leverage the processing power of the application processor
for image authentication.

In order for the scp_ramfw firmware image to be loaded per the boot flow
described above, a minimal application processor firmware needs to be available.
This typically consists of at least two images:

- bl1: First-stage bootloader stored in the system ROM
- bl2: Second-stage bootloader loaded by bl1, responsible for loading the
    scp_ramfw firmware image and other application processor firmware images
    into system RAM

For more information about application processor firmware images and how to
build them, please refer to the [Arm Trusted Firmware-A user guide](https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/user-guide.rst).

In order for the bl2 firmware image and the scp_ramfw firmware image to be made
available to their respective loaders, they must be packaged in a Firmware
Image Package (FIP). Please refer to the Arm Trusted Firmware-A user guide for
instructions on building FIP packages.

To run the boot flow described above, use:

    $> <path to the SGM platform FVP> \
        -C soc.pl011_uart0.out_file=./ap.txt \
        -C soc.pl011_uart1.out_file=./scp.txt \
        -C css.scp.ROMloader.fname=<path to scp_romfw firmware image> \
        -C css.trustedBootROMloader.fname=<path to bl1 firmware image> \
        -C board.flashloader0.fname=<path to FIP> \
        -C soc.pl011_uart1.unbuffered_output=1 \
        -C soc.pl011_uart0.unbuffered_output=1

Note:
    - The application processor firmware images can be built using the [Arm
Platforms deliverables](https://community.arm.com/dev-platforms/w/docs/304/arm-platforms-deliverables).
See the following section.

Booting up to the Linux prompt on Arm platforms
-----------------------------------------------

The [Arm Platforms deliverables](https://community.arm.com/dev-platforms/w/docs/304/arm-platforms-deliverables)
provide a set of source code bases and prebuilt binaries of a fully bootable
Linux software stack on supported Arm platforms.

This section explains how to update the SCP-firmware binaries once the full
software stack has been fully built for a given configuration.

To retrieve, build and run the software stack for a given Arm platform, please
refer to the [Linaro instructions](https://community.arm.com/dev-platforms/w/docs/304/linaro-software-deliverables)

Once the software stack has been retrieved, the SCP/MCP source code can be found
in the 'scp' directory at the root of the workspace.

To rebuild the SCP-firmware binaries without rebuilding the whole software
stack, at the root of the framework, use:

    $> ./build_scripts/build-scp.sh build

Once the software stack has been fully built, use the following to update the
SCP binaries in the software package to be run:

    $> ./build_scripts/build-all.sh package

As an example, to run the software stack, on the SGM-775 FVP:

    $> export MODEL=/path/to/where/you/separately/installed/FVP_CSS_SGM-775
    $> cd ./model-scripts/sgm775
    $> ./run_model.sh -t sgm775
