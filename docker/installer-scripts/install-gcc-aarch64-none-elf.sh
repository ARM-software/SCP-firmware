#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set -e

tool_dir=$1
version=$2
hostarch=$(uname -m)

toolchain="gcc-arm-${version}-${hostarch}-aarch64-none-elf"
url="https://developer.arm.com/-/media/Files/downloads/gnu-a/${version}/binrel/${toolchain}.tar.xz"

echo -e "Installing ${toolchain}\n"

# Create target folder
mkdir -p ${tool_dir}

# Download
wget -nv ${url}

# Extract
tar -xf ${toolchain}.tar.xz -C ${tool_dir} --strip-components=1

# Clean up
rm ${toolchain}.tar.xz
