# Architecture Support

## Overview

Architectural configuration comes under the `arch` directory. This area provides
not only instructions for building the architecture library, but for configuring
any architecture-dependent behaviour required by the framework.

## Structure

The structure of this directory is like so:

    .
    |-- <vendor>
    |   |-- <architecture>
    |   |   |-- arch.mk
    |   |   |-- include
    |   |   |   `-- arch_*.h
    |   |   `-- src
    |   |       |-- arch.ld.S
    |   |       |-- arch.scatter.S
    |   |       |-- arch_*.S
    |   |       |-- arch_*.c
    |   |-- include
    |   |   `-- arch_*.h
    |   |-- src
    |   |   `-- arch_*.c
    |   `-- vendor.mk

### Vendors and architectures

The _vendor_ refers to the umbrella architecture group. For Arm-based
architectures this is "arm".

The _architecture_ refers to the instruction set architecture of the target
platform.

### Build system integration

`vendor.mk` is included by the build system when building the architecture
library, and also when building the firmware. This allows you to configure
sources to compile in the architecture library, and library dependencies
(if any) when linking the final firmware binary.

`arch.ld.S` and `arch.scatter.S` also provide the linker scripts for the final
firmware. They are automatically used by the build system if not targeting the
*none* vendor (which one is chosen is based on the linker in use).
