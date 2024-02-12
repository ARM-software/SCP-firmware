#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set -e

tool_dir=$1
version=$2

url="https://github.com/doxygen/doxygen.git"

echo -e "Installing Doxygen tool version: ${version}\n"

# Install dependencies

apt-get install -y --no-install-recommends \
        flex \
        bison

# Create target folder
mkdir -p ${tool_dir}

# Download
git clone "${url}" --depth 1 --branch \
        "Release_$(echo $version | sed 's/\./_/g')" ${tool_dir}/source

# Build
mkdir -p ${tool_dir}/source/build
pushd ${tool_dir}/source/build
cmake -G "Unix Makefiles" ..
make
make install/local

popd
