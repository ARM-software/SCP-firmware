#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set -e

tool_dir=$1
version=$2

url="https://github.com/danmar/cppcheck.git"

echo -e "Installing cppcheck tool version: ${version}\n"

# Create target folder
mkdir -p ${tool_dir}
pushd ${tool_dir}

# Download
git clone "${url}" --depth 1 --branch "${version}"  source

# Build
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX=${tool_dir} \
        -DFILESDIR=${tool_dir} ./source
cmake --build . -- install

popd
