#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

"""
Build and check framework
This script runs "CC=gcc make -f Makefile.cmake test" and performs all
frameworks tests.
"""

import sys
import subprocess


def banner(text):
    columns = 80
    title = " {} ".format(text)
    print("\n\n{}".format(title.center(columns, "*")))


def main():
    banner("Build and run framework tests")

    result = subprocess.Popen(
        "CC=gcc make -f Makefile.cmake test",
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    (stdout, stderr) = result.communicate()

    print(stdout.decode())

    if stderr:
        print(stderr.decode())
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())
