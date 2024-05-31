#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import argparse
import check_build
import check_copyright
import check_doc
import check_spacing
import check_tabs
import check_pycodestyle
import check_utest
import os
import signal
import subprocess
import sys
from product import Product, Build, Parameter
from typing import List, Tuple
from utils import Results, banner

#
# Default products build configuration file
#
PRODUCTS_BUILD_FILE_DEFAULT = './tools/products_build.yml'

#
# Default build output directory
#
BUILD_OUTPUT_DIR_DEFAULT = '/tmp/scp/build'

code_validations = [
    check_copyright,
    check_spacing,
    check_tabs,
    check_doc,
    (check_utest, (False, 'fwk')),
    (check_utest, (False, 'mod')),
    (check_utest, (False, 'prod')),
    check_pycodestyle,
]


def check_argument(results, check):
    if isinstance(check, tuple):
        print(check[0], check[1])
        result = check[0].run(*check[1])
        test_name = check[0].__name__.split('_')[-1] + " " + check[1][1]
    else:
        result = check.run()
        test_name = check.__name__.split('_')[-1]
    results.append(('Check {}'.format(test_name), 0 if result else 1))


def code_validation(checks: list) -> List[Tuple[str, int]]:
    banner('Code validation')
    results = Results()
    for check in checks:
        check_argument(results, check)
    return results


def print_results(results: List[Tuple[str, int]]) -> Tuple[int, int]:
    banner('Tests summary')
    total_success = 0
    for result in results:
        if result[1] == 0:
            total_success += 1
            verbose_result = "Success"
        else:
            verbose_result = "Failed"
        print("{}: {}".format(result[0], verbose_result))

    assert total_success <= len(results)
    return (total_success, len(results))


def analyze_results(success: int, total: int) -> int:
    print("{} / {} passed ({}% pass rate)".format(success, total,
                                                  int(success * 100 / total)))
    return 1 if success < total else 0


def main(config_file: str, ignore_errors: bool, log_level: str,
         output_path: str):
    results = Results()

    results.extend(code_validation(code_validations))
    if not ignore_errors and results.errors:
        print('Errors detected! Excecution stopped')
        return analyze_results(*print_results(results))

    banner('Test building products')
    results.extend(check_build.build_all(config_file, ignore_errors,
                   log_level, output_path))

    return analyze_results(*print_results(results))


def parse_args():
    parser = argparse.ArgumentParser(
        description='Perform basic checks to SCP-Firmware and build for all \
                     supported platforms, modes and compilers.')

    parser.add_argument('-c', '--config-file', dest='config_file',
                        required=False, default=PRODUCTS_BUILD_FILE_DEFAULT,
                        type=str, action='store', help=f'Products build \
                        configuration file, if it is not provided, the \
                        default location is {PRODUCTS_BUILD_FILE_DEFAULT}')

    parser.add_argument('-i', '--ignore-errors', dest='ignore_errors',
                        required=False, default=False, action='store_true',
                        help='Ignore errors and continue testing.')

    parser.add_argument('-ll', '--log-level', dest='log_level',
                        required=False, default="", type=str,
                        action='store', help='Build every product with the \
                        specified log level.')

    parser.add_argument('-bod', '--build-output-dir', dest='output_path',
                        required=False, default=BUILD_OUTPUT_DIR_DEFAULT,
                        type=str, action='store', help='Parent directory of \
                        the "build-output" directory, the one were the build \
                        logs will be stored in.\nIf bod is not given, the \
                        default location is /tmp/scp/ build-output')

    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    sys.exit(main(args.config_file, args.ignore_errors, args.log_level,
                  args.output_path))
