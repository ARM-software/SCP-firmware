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

This software has been tested on Ubuntu 20.04 LTS (64-bit).

## Prerequisites

To build the SCP/MCP firmware for a target product, the following tools are
required:

- [GNU Make] (*4.2* or later)
- [CMake] (*3.18.4* or later)
- [Python 3] (*3.7.9* or later)

[GNU Make]: https://www.gnu.org/software/make/
[Python 3]: https://www.python.org/downloads/release/python-379/
[CMake]: https://cmake.org/

Additionally, the firmware may be built using one of three compilers:

- [GNU Arm Embedded Toolchain] (*10.3-2021.10* or later)
- [Arm Compiler 6] (*6.13* or later)
- [LLVM Toolchain] (*13.0.1* or later)

[GNU Arm Embedded Toolchain]:
https://developer.arm.com/open-source/gnu-toolchain/gnu-rm
[Arm Compiler 6]:
https://developer.arm.com/tools-and-software/embedded/arm-compiler/downloads/version-6
[LLVM Toolchain]: https://releases.llvm.org/download.html


If building using the LLVM toolchain, the [GNU Arm Embedded Toolchain] is also
required for the Arm standard library and headers that ship with it. When
building for a ARMv7 target the respective Arm [Compiler-RT builtins] are
also required.

[Compiler-RT builtins]: https://compiler-rt.llvm.org/

The following tools are recommended but not required:

- [Doxygen] (*1.8.13* or later): Required to generate supporting documentation
- [AArch64 GCC toolchain] (*6.2.1* or later): To build the Trusted Firmware in
order to run the tests suites
- [ARM GCC GNU-A toolchain] (*7.4.0* or later): Required to build framework
tests that run on the host system
- [lcov] (*1.13* or later): Required to run unit test framework
- [cppcheck] (*1.90*): Required during build process to check the code
- [ninja-build] (*1.10.0* or later): Default build system to compile the project
UNIX-Make is a suitable alternative if preferred.
- [clang-format] (*10.0.0* or later): Automatic code formatter.

[Doxygen]: http://www.doxygen.nl/
[AArch64 GCC toolchain]:
http://releases.linaro.org/components/toolchain/binaries
[ARM GCC GNU-A toolchain]:
https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads
[Cppcheck]: https://github.com/danmar/cppcheck/tree/1.90
[Ninja-build]: https://ninja-build.org/
[Clang-format]: https://clang.llvm.org/docs/ClangFormat.html

If building for an Arm FVP platform, you will need to ensure you have [the
relevant FVP].

[the relevant FVP]: https://developer.arm.com/products/system-design/fixed-virtual-platforms

The FVPs also have a soft dependency on the following tools:

- [xterm]: Required to view UART output

[xterm]: https://invisible-island.net/xterm/

The instructions provided as a part of this guide assume you have [Git]
(*2.17.1* or later) available in your environment.

[Git]: https://git-scm.com/

Installing some of these prerequisites can be done on any standard Debian-based
system with the following:

```sh
sudo apt install \
    build-essential \
    doxygen \
    git \
    python3 \
    python3-pip \
    device-tree-compiler \
    ninja-build \
    lcov
```

For setting up the build system and its requirements (CMake among them), visit
the cmake_readme.md file.

For the FVP prerequisites:

```sh
sudo apt install xterm
```

For code style checks in Python scripts (`pip3` needs to be installed):

```sh
pip3 install pycodestyle
pip3 install --upgrade pycodestyle
sudo apt-get install pep8
```

## Cloning the SCP-firmware source code

The SCP-firmware source code can be cloned from the official GitHub repository:

```sh
git clone --recurse-submodules https://github.com/ARM-software/SCP-firmware.git\
 ${SCP_PATH}
```

### Cloning dependencies

Under certain configurations the SCP-firmware has a dependency on the CMSIS-Core
projects, which are part of the [Cortex Microcontroller System Interface
Standard (CMSIS)] software pack. The source tree for this software is
included with the firmware as a Git submodule. You can fetch all submodules from
within the source directory with the following:

[Cortex Microcontroller System Interface Standard (CMSIS)]:
https://www.arm.com/why-arm/technologies/cmsis

```sh
git submodule update --init --recursive
```
This command will also fetch submodules related to unit testing.For more
information please refer to the `unit_test/user_guide.md`
documentation.

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
top-level Makefile.cmake file:

```sh
make -f Makefile.cmake doc
```
## Building SCP-firmware

To build SCP-firmware for a specific product the basic command format for
invoking `make` (from within the source directory) is:

```sh
make -f Makefile.cmake PRODUCT=<PRODUCT> [OPTIONS] [TARGET]
```

For example, to build the RAM firmware for TC2 in debug mode, use the
following:

```sh
make -f Makefile.cmake PRODUCT=tc2 MODE=debug firmware-scp_ramfw
```

The `all` target will be used if `[TARGET]` is omitted, which will build all the
firmware defined by the product.

The `help` target provides further information on the arguments that can be
given:

```sh
make -f Makefile.cmake help
```

The framework includes a suite of tests that validate its core functionality.
If you installed the native GCC prerequisite, these can be run on the host
system using:

```sh
make -f Makefile.cmake test
```

For more guidance and information on the build system, refer to the full set of
documentation included in the *Build System* chapter of the Doxygen-generated
documentation.

### Building with LLVM

When building with the LLVM toolchain, it is mandatory to pass the required
standard library and headers. These are taken from the
[GNU Arm Embedded Toolchain]. According to the desired product and target the
required environment variables differ. The PATH variable of the system must
incorporate path to the required toolchain.

#### ARMv7

When building for an ARMv7 product the sysroot path of the GNU Arm Embedded
Toolchain must be passed under the `SYSROOT` environment variable.

```sh
make -f Makefile.cmake PRODUCT=tc2 \
    LLVM_SYSROOT_CC=/path/to/sysroot
```

The Compiler-RT builtins for baremetal are usually placed in:

```sh
/path/to/clang/resource/dir/lib/baremetal
```

For a LLVM 13 installation on Ubuntu this could be:

```sh
/usr/lib/llvm-13/lib/clang/13.0.1/lib/baremetal
```

You can discover the resource dir of your Clang 13 installation by running:

```sh
clang-13 -print-resource-dir
```

#### ARMv8

When building for an ARMv8 product the sysroot path of the GNU Arm Embedded
Toolchain must be passed under the `SYSROOT` environment variable.

Building example for all of the R-Car targets:

```sh
make -f Makefile.cmake PRODUCT=rcar TOOLCHAIN=GNU \
    LLVM_SYSROOT_CC=/path/to/sysroot
```

## Running the SCP-firmware on Total Compute (TC) platforms

For setting up the environment, installing all dependencies, configuration,
building the system and running it on an FVP, please refer to, and follow, the
[TC2 User Guide]. Bear in mind that the installation process might require root
privileges.

[TC2 User guide]:
https://gitlab.arm.com/arm-reference-solutions/arm-reference-solutions-docs/-/blob/master/docs/totalcompute/tc2/user-guide.rst

The instructions within this section use TC2 BSP only without Android
(buildroot) as an example platform, but they are relevant for all TC platforms.

After setting up the environment it would be desirable to set a SCP-firmware
version, to do that please run:

```sh
cd <tc2_workspace>/src/SCP-firmware
git fetch <remote name/url> <branch/tag/hash commit id>
git checkout FETCH_HEAD
```

### Build all components

```sh
export PLATFORM=tc2
export FILESYSTEM=buildroot
<tc2_workspace>/build-scripts/build-all.sh -p $PLATFORM -f $FILESYSTEM build
```

### Running Buildroot

When all build images are created and deployed you can run the following command
to test the system in a FVP.

```sh
<tc2_workspace>/run-scripts/tc2/run_model.sh -m <model binary path> -d buildroot
```

### Obtaining the TC2 FVP
The TC2 FVP is available to partners for build and run on Linux host
environments. Please contact Arm to have access (support@arm.com).


## Running the SCP firmware on SGI and Neoverse Reference Design platforms

For an introduction to the System Guidance for Infrastructure (SGI) platforms,
please refer to [System Guidance for Infrastructure (SGI)].

[System Guidance for Infrastructure (SGI)]: https://community.arm.com/developer/tools-software/oss-platforms/w/docs/387/system-guidance-for-infrastructure-sgi

For an introduction to the Neoverse Reference Design (RD) platforms, please
refer to [Neoverse Reference Designs].

[Neoverse Reference Designs]: https://developer.arm.com/tools-and-software/development-boards/neoverse-reference-design

The instructions within this section use SGI-575 as an example platform, but
they are relevant for all SGI and Neoverse Reference Design platforms.

### Building the images

The build system generates firmware images per the `product.mk` file associated
with the product. For SGI and Neoverse Reference Design platforms, three
firmware images are built:

- `sgi575-bl1.bin`: SCP ROM firmware image - loads the SCP RAM firmware from NOR
    flash into private SRAM and jumps to it
- `sgi575-bl2.bin`: SCP RAM firmware image - manages the system runtime services
- `sgi575-mcp-bl1.bin`: MCP ROM firmware image

```sh
cd ${SCP_PATH} && \
    make -f Makefile.cmake PRODUCT=sgi575 MODE=debug

export SCP_ROM_PATH=${SCP_PATH}/build/sgi575/GNU/debug/firmware-scp_romfw/bin/\
sgi575-bl1.bin
export SCP_RAM_PATH=${SCP_PATH}/build/sgi575/GNU/debug/firmware-scp_ramfw/bin/\
sgi575-bl2.bin
export MCP_ROM_PATH=${SCP_PATH}/build/sgi575/GNU/debug/firmware-mcp_romfw/bin/\
sgi575-mcp-bl1.bin
```

__Note:__ If building with LLVM, make sure to pass the required environment
variables to `make` as noted in [Building with LLVM](#building-with-llvm).

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

[the Arm Developer documentation]:
https://community.arm.com/developer/tools-software/oss-platforms/w/docs/485/juno

### Building the images

The build system generates firmware images per the `product.mk` file associated
with the product. For Juno platform, three firmware images are built:

- `juno-bl1.bin`: SCP ROM firmware image - handles the transfer of the RAM
    firmware to private SRAM and jumps to it
- `juno-bl1-bypass.bin`: SCP ROM bypass firmware image - an alternative ROM
    firmware that is loaded from an external non volatile on-board memory.
    This binary needs to be used in order to successfully load the SCP RAM
    firmware, and is chain-loaded from the burned-in ROM on the physical board
    (not necessary for the FVP).
- `juno-bl2.bin`: SCP RAM firmware image - manages the system runtime services

```sh
cd ${SCP_PATH} && \
    make -f Makefile.cmake PRODUCT=juno MODE=debug

export SCP_ROM_PATH=${SCP_PATH}/build/juno/GNU/debug/firmware-scp_romfw/\
bin/juno-bl1.bin
export SCP_ROM_BYPASS_PATH=${SCP_PATH}/build/juno/GNU/debug/\
firmware-scp_romfw-byopass/bin/juno-bl1-bypass.bin
export SCP_RAM_PATH=${SCP_PATH}/build/juno/GNU/debug/firmware-scp_ramfw/\
bin/juno-bl2.bin
```

__Note:__ If building with LLVM, make sure to pass the required environment
variables as noted in [Building with LLVM](#building-with-llvm).

### Booting the firmware

In order for the `juno-bl2.bin` firmware image to be loaded, an application
processor secure world firmware needs to be available to load it. Arm maintains
the [Arm Trusted Firmware-A (TF-A)] project, which handles this case. The
remaining instructions assume you are using Trusted Firmware-A.

[Arm Trusted Firmware-A (TF-A)]:
https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git

To boot the SCP firmware on Juno with TF-A, you will need at minimum
three additional images:

- `bl1`: BL1 - first-stage bootloader stored in the system ROM
- `bl2`: BL2 - second-stage bootloader loaded by `bl1`, responsible for handing
    over `scp_bl2` to the SCP
- `fip`: FIP - firmware image package containing `bl2` and `scp_bl2`

The FIP format acts as a container for a number of commonly-used images in the
TF-A boot flow. Documentation for the FIP format can be found in the [TF-A
firmware design documentation].

[TF-A firmware design documentation]:
https://trustedfirmware-a.readthedocs.io/en/latest/design/firmware-design.html?highlight=fip#firmware-image-package-fip

An example command line to build Arm Trusted Firmware-A for AArch64 is given
below. Note that you will need to have installed [the prerequisites for building
Arm Trusted Firmware-A for Juno].

[the prerequisites for building Arm Trusted Firmware-A for Juno]:
https://trustedfirmware-a.readthedocs.io/en/latest/getting_started/docs-build.html


```sh
export TFA_PATH=<your Trusted Firmware-A path>

git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git \
${TFA_PATH}

cd ${TFA_PATH}

make CROSS_COMPILE=aarch64-linux-gnu- DEBUG=1 LOG_LEVEL=30 PLAT=juno \
    CSS_USE_SCMI_SDS_DRIVER=1 SCP_BL2=<path to scp_bl2> \
    BL33=<path to bl33file> bl1 bl2 fip

export BL1_PATH=${TFA_PATH}/build/juno/debug/bl1.bin
export BL2_PATH=${TFA_PATH}/build/juno/debug/bl2.bin
export FIP_PATH=${TFA_PATH}/build/juno/debug/fip.bin

```

Before beginning, please ensure the SD card used for your Juno board has been
set up with a Linaro release software stack. If this is not the case, you can
follow the [Linaro software release instructions] and/or download a new SD card
filesystem from the [Linaro releases page].

[Linaro software release instructions]:
https://git.linaro.org/landing-teams/working/arm/arm-reference-platforms.git/about/docs/juno/user-guide.rst
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

[Getting Started Guide]: https://developer.arm.com/documentation/dui0928/e

## Software stack

Arm provides [a super-project] with guides for building and running a full
software stack on Arm platforms. This project provides a convenient wrapper
around the various build systems involved in the software stack, including for
SCP-firmware.

[a super-project]:
https://git.linaro.org/landing-teams/working/arm/arm-reference-platforms.git/about/docs/user-guide.rst

## Deprecated platforms

This section provides some guidance for deprecating a platform. The code in
this case remains in the repository, but support is no longer provided.
The process involves removing the chosen platform from the build and adding it
to the list of deprecated platforms.

1. Remove the chosen-platform from the "products" list in tools/ci_cmake.py
2. Add the chosen-platform to the "DEPRECATED_PLATFORMS" list in Makefile.cmake
3. Submit a change to this repository
