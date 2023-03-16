#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

"""
Check pycodestyle
This script runs pycodestyle to check all python scripts.
"""

import sys
import subprocess

INCLUDE_DIRECTORIES = "tools/ unit_test/utils"


def banner(text):
    columns = 80
    title = " {} ".format(text)
    print("\n\n{}".format(title.center(columns, "*")))


def main():
    banner("Build and run PyCodeStyle tests")

    result = subprocess.Popen(
        "python3 -m pycodestyle {}".format(INCLUDE_DIRECTORIES),
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    (stdout, stderr) = result.communicate()

    print(stdout.decode())

    if result.returncode != 0:
        print(stderr.decode())
        print('FAILED')
        return 1
    print('SUCCESS')
    return 0


if __name__ == '__main__':
    sys.exit(main())
