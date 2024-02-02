#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

"""
Check for missing documentation.
This script runs 'make -f Makefile.cmake doc' and checks for any
output on stderr, where the Doxygen tool outputs any warnings
about undocumented components
"""

import sys
import subprocess
from utils import banner


def run():
    print(banner('Checking for undocumented code'))

    result = subprocess.Popen(
        'make -f Makefile.cmake doc',
        shell=True,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE)

    (stdout, stderr) = result.communicate()

    if result.returncode != 0:
        print(stderr.decode())
        print('Error checking for undocumented code.')
        return False

    print('The codebase is fully documented.')
    return True


def main():
    return 0 if run() else 1


if __name__ == '__main__':
    sys.exit(main())
