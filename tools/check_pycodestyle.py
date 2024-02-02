#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

"""
Check pycodestyle
This script runs pycodestyle to check all python scripts.
"""

import sys
import subprocess
from utils import banner

INCLUDE_DIRECTORIES = 'tools/ unit_test/utils'


def run():
    print(banner('Run PyCodeStyle tests'))

    result = subprocess.Popen(
        'python -m pycodestyle {}'.format(INCLUDE_DIRECTORIES),
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    (stdout, stderr) = result.communicate()

    print(stdout.decode())

    if result.returncode != 0:
        print(stderr.decode())
        print('Run PyCodeStyle tests failed.')
        return False

    print('Run PyCodeStyle tests succedded.')
    return True


def main():
    return 0 if run() else 1


if __name__ == '__main__':
    sys.exit(main())
