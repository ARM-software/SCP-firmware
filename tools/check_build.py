#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import argparse
import os
import sys
import subprocess
from product import Product, Build, Parameter
from typing import List, Tuple
from utils import Results, banner

"""
    Check for product build
"""

#
# Default products build configuration file
#
PRODUCTS_BUILD_FILE_DEFAULT = './tools/config/check_build/' \
                              'default_products_build.yml'

#
# Default build output directory
#
BUILD_OUTPUT_DIR_DEFAULT = '/tmp/scp/build'


def do_build(build_info: List[Build], output_path: str) -> \
        List[Tuple[Build, subprocess.Popen]]:
    build_status: List[Tuple[Build, subprocess.Popen]] = []
    results = Results()

    if not os.path.exists(output_path):
        os.makedirs(output_path)

    files = []
    for build in build_info:
        file_path = os.path.join(output_path, build.file_name())
        files.append(open(file_path, "w", encoding="utf-8"))

        build_cmd = build.command(output_path)

        build_id = subprocess.Popen(
                                build_cmd,
                                shell=True,
                                stdout=files[-1],
                                stderr=subprocess.STDOUT)

        build_status.append((build, build_id))
        print(f'Test building: \n{build_cmd}')
        print(f'Log file: \n\t{build.file_name()}')
        print('-----------')

    for i, (build, build_id) in enumerate(build_status):
        build_id.communicate()
        results.append((build.tag(), build_id.returncode))
        files[i].close()

    return results


def build_products(config_file: str,
                   ignore_errors: bool,
                   log_level: str,
                   products: List[str],
                   output_path: str):

    banner('Test building products')
    results = Results()

    try:
        with open(config_file, 'r') as file:
            all_products = Product.from_yaml(file)
    except IOError:
        print(f'Error opening {config_file} file')
        results.append(("Opening yaml file", 1))
        return results

    if products:
        products_to_build = [product for product in all_products
                             if product.name in products]
    else:
        products_to_build = all_products

    for product in products_to_build:
        if log_level != "":
            product.log_level = Parameter(log_level)
        builds = product.builds
        results.extend(do_build(builds, output_path))
        if not ignore_errors and results.errors:
            print('Errors detected! Excecution stopped')
            break

    return results


def parse_args(argv, prog_name):
    parser = argparse.ArgumentParser(
        prog=prog_name,
        description='Perform basic SCP-Firmware build for all \
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
                        type=str, action='store', help=f'Parent directory of \
                        the "build-output" directory, the one where the build \
                        logs will be stored in.\n If build output directory \
                        is not given, the default location is \
                        {BUILD_OUTPUT_DIR_DEFAULT}')

    parser.add_argument('-p', '--products', dest='products',
                        required=False, default=[], type=str, nargs='+',
                        help='Specify one or more products to build for. \
                        Accepts a space separated list')

    return parser.parse_args(argv)


def main(argv=[], prog_name=''):
    args = parse_args(argv, prog_name)

    results = Results()
    results.extend(build_products(args.config_file,
                                  args.ignore_errors,
                                  args.log_level,
                                  args.products,
                                  args.output_path))
    print(results)
    return 0 if not results.errors else 1


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:], sys.argv[0]))
