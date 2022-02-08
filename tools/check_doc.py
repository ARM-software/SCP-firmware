#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

"""
Check for missing documentation.
This script runs "make -f Makefile.cmake doc" and checks for any
output on stderr, where the Doxygen tool outputs any warnings
about undocumented components
"""

import sys
import subprocess


def main():
    print("Checking for undocumented code...")

    result = subprocess.Popen(
        "make -f Makefile.cmake doc",
        shell=True,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE)

    (stdout, stderr) = result.communicate()

    if stderr:
        print(stderr.decode())
        return 1

    print("The codebase is fully documented.")
    return 0


if __name__ == '__main__':
    sys.exit(main())
