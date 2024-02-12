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

toolchain="gcc-arm-none-eabi-${version}-${hostarch}-linux"
url="https://developer.arm.com/-/media/Files/downloads/gnu-rm/${version}/${toolchain}.tar.bz2"

echo -e "Installing ${toolchain}\n"

# Create target folder
mkdir -p ${tool_dir}

# Download
wget -nv ${url}

# Extract
tar -xf ${toolchain}.tar.bz2 -C ${tool_dir} --strip-components=1

# Clean up
rm ${toolchain}.tar.bz2
