#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

"""
Build and check framework
This script runs 'CC=gcc make -f Makefile.cmake fwk_test' and performs all
frameworks tests.
"""

import sys
import subprocess
from utils import banner


def run():
    print(banner('Build and run framework tests'))

    result = subprocess.Popen(
        'CC=gcc make -f Makefile.cmake fwk_test',
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    (stdout, stderr) = result.communicate()

    print(stdout.decode())

    if result.returncode != 0:
        print(stderr.decode())
        print('Build and run framework tests failed.')
        return False

    print('Build and run framework tests succedded.')
    return True


def main():
    return 0 if run() else 1


if __name__ == '__main__':
    sys.exit(main())
