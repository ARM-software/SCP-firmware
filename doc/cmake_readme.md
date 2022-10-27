# CMake Introduction

The SCP-Firmware project currently uses the CMake build system.
Please follow the instructions below to build the SCP firmware
using CMake.

## Prerequisites

Please follow the prerequisites from [user_guide.md](user_guide.md) 

## Building
Make sure you have updated submodule in the repo
```sh
$ git submodule update --init
```

Unlike the existing build system, CMake only allows the building of
one firmware target at a time e.g

```sh
cmake -B /tmp/build  -DSCP_FIRMWARE_SOURCE_DIR:PATH=juno/scp_romfw
```

This will configure cmake to build firmware for Juno platform scp_romfw firmware.
where
- `/tmp/build`: is the directory where the build will be generated.
- `juno/scp_romfw`: is the firmware to build for the platform.

```sh
cmake --build /tmp/build    # will build the configured cmake
```

In some case like running SCP-firmware with OP-TEE, the firmware is a
static library that is then linked to a larger binary. Instead of providing all
the static libraries that have been built by cmake and which can change with the
configuration of your product, you can gather all of them in a single one.

> ```sh
> cmake --build /tmp/build --target ${SCP_FIRMWARE_TARGET}-all
> will build the configured cmake and append all static librairies in
> lib${SCP_FIRMWARE_TARGET}-all.a
> ```

For ease of building the product, the make wrapper can be used with the below
command to build all firmwares for a product

```sh
$ make -f Makefile.cmake PRODUCT=juno
```

By default all the binaries will be located at `./build/<platform>` directory.
e.g. in above case it will be under `./build/juno directory`.

The above command accepts a number of options. Execute the command below to get
the available options.

```sh
$ make -f Makefile.cmake help
```

## Build and execute framework and module unit tests
See below to build and execute tests

```sh
$ make -f Makefile.cmake test
```

Alternatively, execute just framework or module tests

```sh
$ make -f Makefile.cmake fwk_test
```

```sh
$ make -f Makefile.cmake mod_test
```

See unit_test/user_guide.md for more information on configuring
module tests.

> **LIMITATIONS** \
> ArmClang toolchain is supported but not all platforms are working.

> **NOTE**: Read below documentation for advanced use of development environment
> and CMake build options.

## Build configuration options
CMake provides a feature to configure certain setting to generate required
build. SCP firmware uses this feature to configure various build options
for a firmware target(e.g. `scp_ramfw` or `scp_romfw`). At present the following
few common options can be configured.

- `SCP_ENABLE_NOTIFICATIONS`: Enable/disable notifications within SCP firmware.

- `SCP_ENABLE_SCMI_NOTIFICATIONS`: Enable/disable SCMI notifications.

- `SCP_ENABLE_RESOURCE_PERMISSIONS`: Enable/disable resource permissions
  settings.

- `SCP_ENABLE_PLUGIN_HANDLER`: Enable the Performance Plugin handler extension.

- `SCP_TARGET_EXCLUDE_BASE_PROTOCOL`: Exclude Base Protocol functionality from
  the SCMI Module.

- `SCP_ENABLE_SUB_SYSTEM_MODE`: Enable the execution as a sub-system.

- `SCP_ENABLE_SCMI_SENSOR_EVENTS`: Enable/disable SCMI sensor events.

- `SCP_ENABLE_SCMI_SENSOR_V2`: Enable/disable SCMI sensor V2 protocol support.

- `SCP_ENABLE_SENSOR_TIMESTAMP`: Enable/disable sensor timestamp support.

- `SCP_ENABLE_SENSOR_MULTI_AXIS`: Enable/disable sensor multi axis support.

- `SCP_ENABLE_SCMI_RESET`: Enable/disable SCMI reset.

- `SCP_ENABLE_CLOCK_TREE_MGMT`: Enable/disable clock tree management support.

- `SCP_ENABLE_FAST_CHANNELS`: Enable/disable Fast Channels support. This
  option should be enabled/disabled by the use of a platform specific setting
  like `SCP_ENABLE_SCMI_PERF_FAST_CHANNELS`.

It can also be used to provide some platform specific settings.
e.g. For ARM Juno platform. See below

- `SCP_ENABLE_SCMI_PERF_FAST_CHANNELS`: Enable/disable Juno SCMI-perf
  Fast channels.

- `SCP_ENABLE_DEBUG_UNIT`: Enable/disable Juno debug unit.

- `SCP_ENABLE_STATISTICS`: Enable/disable Juno statistics.

## Modifying build options
There are a number of ways to configure a specific option.

- Using cmake command line option

> ```sh
> cmake -B /tmp/build  -DSCP_FIRMWARE_SOURCE_DIR:PATH=juno/scp_ramfw\
> -DSCP_ENABLE_DEBUG_UNIT=TRUE
> ```
> It will configure cmake to build firmware for Juno platform `scp_ramfw`
> firmware. with debug unit enabled
> where:
> - `/tmp/build`:  is the directory where the build will be generated.
> - `juno/scp_romfw`: is the firmware to build for the platform.
>
> ```sh
> $ cmake --build /tmp/build    # will build the configured cmake
> ```

- Using ccmake or cmake-gui

> ```sh
> $ ccmake -B /tmp/build  -DSCP_FIRMWARE_SOURCE_DIR:PATH=juno/scp_ramfw
> ```
> It will configure cmake to build firmware for Juno platform scp_ramfw
> firmware with debug unit enabled
> where:
> * `/tmp/build`:  is the directory where the build will be generated.
> * `juno/scp_romfw`: is the firmware to build for the platform.
>
> It opens a curses based UI. If `/tmp/build` is not present i.e. if this
> is the first time the configuration is generated, select `'c'` (Configure)
> and then modify the options as desired. After selecting the options
> select `'c'` (Configure) and `'g'`(Generate) to generate the build system.
>
> ```sh
> $ cmake --build /tmp/build    # will build the configured cmake
> ```
>
> Build options can subsequently tuned using below command
> ```sh
> $ ccmake /tmp/build
> ```

- Using default value set for a specific option in `Firmware.cmake`.
>
> Every SCP firmware specific option(e.g. `SCP_ENABLE_XXX`) has a corresponding
> `_INIT` variable in respective `Firmware.cmake` and can be modified before
> build generation
>
> e.g.
> For Arm Juno platform `scp_ramfw/Firmware.cmake` following value can be edited
> manually before build configuration is generated.
>
> ```cmake
> set(SCP_ENABLE_DEBUG_UNIT_INIT FALSE)
> ```
>
> **NOTE** In this method, if value needs to be re-modified then old build
> folder must be manually deleted. Subsequent re-run without deleting old
> configuration will not update the earlier configured value, See CACHE
> variables in CMake documentation.

**NOTE**: Enabling/disabling option may result in inclusion or exclusion of
a particular module in the firmware.
e.g. See module/resource_perms/Module.cmake and note following
```cmake
if(SCP_ENABLE_RESOURCE_PERMISSIONS)
   list(APPEND SCP_MODULES "resource-perms")
endif()
```
The above code will include `resource-perms` module in the firmware only if
`SCP_ENABLE_RESOURCE_PERMISSIONS` is enabled. This also means, define
`BUILD_HAS_MOD_RESOURCE_PERMS` will available only if this option is enabled.

# Development Environments {#development-environments}

Along with core CMake build system, also provided is a option to generate
a build environment, please see details below.

## Vagrant (recommended) {#vagrant}

> **NOTE**: If you're unfamiliar with Vagrant, we recommend you read the brief
> introduction found [here][Vagrant].

Vagrant is an open-source software product for building and maintaining portable
virtual software development environments. The `SCP-firmware `project offers a
Vagrant configuration based on the [Docker] provider, and so you will need to
have them both installed:

- Install [Docker](https://docs.docker.com/get-docker)
- Install [Vagrant](https://www.vagrantup.com/downloads)

> **NOTE**: Vagrant and Docker are generally both available through system
> package managers:
>
> - Ubuntu and other Debian-based Linux distributions:
>   https://docs.docker.com/engine/install/ubuntu/
>
> ```sh
> $ sudo apt install vagrant
> ```
>

When using Vagrant, there are no additional prerequisites for the host system,
as all build and quality assurance tools are packaged with the container.

[Docker]: https://www.docker.com/why-docker
[Vagrant]: https://www.vagrantup.com/intro

### Interactive Development {#interactive-vagrant-development}

You can bring up an interactive development environment by simply running the
following:

```sh
$ vagrant up
Bringing machine 'default' up with 'docker' provider...
==> default: Machine already provisioned. Run `vagrant provision` or use the
`--provision`
==> default: flag to force provisioning. Provisioners marked to run always will
still run.
```

> **NOTE**: The Docker container image will be built the first time you run
> this, which can take a while. Be patient - it won't happen again unless you
> modify the Dockerfile.

The project working directory will be mounted within the container as
`/scp-firmware`.

You can then connect to the embedded SSH server as the non-root user `user`
with:

```sh
$ vagrant ssh
```

You will have access to `sudo` from within the container, and Vagrant will
persist any changes to the container even if you halt it. If you need to rebuild
the container for any reason, like if you have made changes to the Dockerfile,
you can rebuild the development environment with:

```sh
$ vagrant reload
```

Do note, however, that reloading the development environment will rebuild it
from scratch.

### Running Commands {#running-vagrant-commands}

If you simply wish to invoke a command from within the container, you may also
use Vagrant's [`docker-run`] command, e.g.:

```sh
$ vagrant docker-run -- pwd
==> default: Image is already built from the Dockerfile. `vagrant reload` to
rebuild.
==> default: Creating the container...
    default:   Name: git_default_1601546529_1601546529
    default:  Image: b7c4cbfc3534
    default:    Cmd: pwd
    default: Volume: /tmp/tmp.cGFeybHqFb:/vagrant
    default:
    default: Container is starting. Output will stream in below...
    default:
    default: /vagrant
```

[`docker-run`]: https://www.vagrantup.com/docs/providers/docker/commands#docker-run

## Visual Studio Code Development Containers {#vscode-containers}

If you use Visual Studio Code, you may also work directly within a
pre-configured development container. See the [official tutorial] if you are
unfamiliar with the process of developing within containers.

[official tutorial]: https://code.visualstudio.com/docs/remote/containers-tutorial

## Docker {#docker}

> **NOTE**: Using Docker in SCP build might show some errors.

The SCP-firmware project includes a [`Dockerfile`] which can be used to set up
an environment containing all of the necessary prerequisites required to build
and test a firmware.

This Dockerfile has four variants:

- `ci`: A continuous integration variant, which provides the tooling required
  for automating builds and quality assurance processes.
- `jenkins`: A Jenkins-specific continuous integration variant, which includes
  additional steps required to use it from Jenkins.
- `dev`: A development variant, which includes additional tools for developers
  accessing the container directly.
- `vagrant`: A Vagrant variant, which includes an SSH server and a workspace
  more familiar to Vagrant users.

We *highly* recommend using [Vagrant](#vagrant) to manage your development
environment, but in case you must do so directly through Docker, you can build
the development container with the following:

```sh
$ docker build -t scp-firmware --target=dev "./docker"
```

You can then begin an interactive login shell with:

```sh
$ docker run -v $(pwd):/scp-firmware -v ~/.gitconfig:/home/user/.gitconfig \
    -e TERM -e ARMLMD_LICENSE_FILE -it scp-firmware /bin/bash
```

Alternatively, you can run commands directly from within the container with:

```sh
$ docker run -v $(pwd):/scp-firmware -v ~/.gitconfig:/home/user/.gitconfig \
      -e TERM -e ARMLMD_LICENSE_FILE -it scp-firmware pwd

```

[`Dockerfile`]: ./docker/Dockerfile

# Getting Started {#getting-started}

## Prerequisites {#prerequisites}

In order to build, validate and test the full suite of in-tree firmware targets,
you will need the following (adjust for your desired platform):

- [cmake-format 0.6.13](https://pypi.org/project/cmake-format/0.6.13/#files)
- [CMake 3.18.3](https://github.com/Kitware/CMake/releases/tag/v3.18.3)
- [Markdownlint 0.11.0](https://rubygems.org/gems/mdl/versions/0.11.0)
- [Python 3.8.2](https://www.python.org/downloads/release/python-382)
- [yamllint 1.25.0](https://pypi.org/project/yamllint/1.25.0/#files)

## Basics {#basics}

The SCP-firmware project uses CMake, a build system generator for C-like
languages, to configure and generate its build system. This section is
dedicated to getting you familiarized with the basics of our CMake build system
configuration.

For basic usage instructions on CMake, see [Running CMake].

[Running CMake]: https://cmake.org/runningcmake

## Configuration

### Firmware

> **NOTE:** Unlike in previous generations of the build system, there is no
> "product" concept, and you select the firmware directly. While this prevents
> you from building all the firmware images of a product at once, it allows you
> to configure firmware images individually.

In order to begin with building a firmware, you will need to first provide to
the build system a path to the firmware you wish to build. This is done through
the `SCP_FIRMWARE_SOURCE_DIR` cache variable, and represents a directory
containing a `Firmware.cmake` file, which the build system will load to
configure the firmware target. If you're using `cmake` on the command line, this
should look like the following:

```sh
$ cmake "${SCP_SOURCE_DIR}" -B "${SCP_BUILD_DIR}" \
  -DSCP_FIRMWARE_SOURCE_DIR:PATH="${SCP_PRODUCT}/${SCP_FIRMWARE}"
  ...
```

`SCP_FIRMWARE_SOURCE_DIR` may also be a path relative to the root project
directory.

### Variables

> **NOTE:** If you are using a GUI-based tool, how you configure these variables
> may differ from how this section describes. For instance, `cmake-gui` and
> `cmake` both present the options you may configure by default, and you do not
> need to do anything else. If you are using an IDE, you may need to modify
> the generated `CMakeCache.txt` file, or your IDE may offer an integrated way
> to modify these variables.

The various configuration variables can be listed on the command line with:

```sh
$ cmake "${SCP_SOURCE_DIR}" -LAH
```

This will give you a list of the options available to you as well as a helpful
description and their type.

You can override the default values for any of the settings you see with the
`-D` CMake option:

```sh
$ cmake "${SCP_SOURCE_DIR}" -B "${SCP_BUILD_DIR}" \
  -D${VARIABLE_NAME}:${VARIABLE_TYPE}="${VARIABLE_VALUE}"
  ...
```

> **NOTE:** If you do not see a variable that you expect to see, it is likely
> because the firmware has forcibly overridden it.

### Toolchain

If you wish to adjust the toolchain used to build the firmware, you may provide
the `SCP_TOOLCHAIN` cache variable. Toolchain support is on a per-firmware
basis, and the toolchains supported by the firmware are given by
`Toolchain-${SCP_TOOLCHAIN}.cmake` files found in the firmware.

When `SCP_TOOLCHAIN` is set as `Clang` `SCP_LLVM_SYSROOT_CC` must be defined.

For example, a firmware supporting both GCC and Arm Compiler 6 may offer a `GNU`
toolchain and an `ArmClang` toolchain (`Toolchain-GNU.cmake` and
`Toolchain-ArmClang.cmake`). In this situation, for GCC you might use:

```sh
$ cmake "${SCP_SOURCE_DIR}" -B "${SCP_BUILD_DIR}" \
  -DSCP_TOOLCHAIN:STRING="GNU"
  ...
```

Or for Arm Compiler 6:

```sh
$ cmake "${SCP_SOURCE_DIR}" -B "${SCP_BUILD_DIR}" \
  -DSCP_TOOLCHAIN:STRING="ArmClang"
  ...
```

Alternatively, if you wish to use a [custom toolchain file], you may provide
[`CMAKE_TOOLCHAIN_FILE`]:

```sh
$ cmake "${SCP_SOURCE_DIR}" -B "${SCP_BUILD_DIR}" \
  -DCMAKE_TOOLCHAIN_FILE:FILEPATH="${SCP_TOOLCHAIN_FILE}"
  ...
```

[custom toolchain file]: https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html
[`CMAKE_TOOLCHAIN_FILE`]: https://cmake.org/cmake/help/latest/variable/CMAKE_TOOLCHAIN_FILE.html

> **NOTE:** An out-of-tree firmware may be configured per these instructions by
> providing a firmware directory outside of the project source directory.

## Quality Assurance {#qa}

The SCP-firmware project has adopted a number of quality assurance tools in
order to programmatically measure and increase code quality, and most of these
tools can be invoked directly through the build system for convenience with the
following:

```sh
$ cmake --build <build directory> --target <target>
```

Or, alternatively, directly through the generated build system. For instance,
if you configured the project with the *Unix Makefiles* generator:

```sh
$ make -C <build directory> <target>
```

The following validation targets are supported:

- `check`: Runs all linting and formatting checks
  - `lint`: Runs all linting checks
    - `lint-cmake`: Runs CMake linting checks
    - `lint-python`: Runs Python linting checks
    - `lint-markdown`: Runs Markdown linting checks
    - `lint-yaml`: Runs YAML linting checks
  - `check-cmake`: Runs CMake formatting checks
  - `check-yaml`: Runs YAML formatting checks
  - `check-c`: Runs C formatting checks
- `format`: Formats all code
  - `format-cmake`: Formats all CMake code
  - `format-yaml`: Formats all YAML code
  - `format-c`: Formats all C code
