#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

"""
Build and check unit tests for modules.
This script runs 'CC=gcc make -f Makefile.cmake mod_test' and performs all
modules' unit tests.
"""

import argparse
import sys
import subprocess
from utils import banner


def run(coverage=False):
    print(banner('Build and run modules\' unit tests'))

    extra_args = f'ENABLE_COVERAGE={"y" if coverage else "n"}'
    result = subprocess.Popen(
        f'CC=gcc make -f Makefile.cmake mod_test {extra_args}',
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    (stdout, stderr) = result.communicate()

    print(stdout.decode())

    if result.returncode != 0:
        print(stderr.decode())
        print('Build and run modules\' unit tests failed.')
        return False

    print('Build and run modules\' unit tests succedded.')
    return True


def parse_args(argv, prog_name):
    parser = argparse.ArgumentParser(
        prog=prog_name,
        description='Build and run modules\' unit tests')

    parser.add_argument('-c', '--coverage', dest='coverage',
                        required=False, default=False, action='store_true',
                        help='Enable code coverage reporting.')

    return parser.parse_args(argv)


def main(argv=[], prog_name=''):
    args = parse_args(argv, prog_name)
    return 0 if run(args.coverage) else 1


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:], sys.argv[0]))
