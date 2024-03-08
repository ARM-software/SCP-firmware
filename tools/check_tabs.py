#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
"""
    Check for tabs in the source code.
"""
import argparse
import os
import shutil
import subprocess
import sys
import tempfile
import glob
from utils import banner, get_filtered_files

#
# Directories to exclude
#

# Exclude all mod_test 'mocks' directories
UNIT_TEST_MOCKS = glob.glob('module/**/test/**/mocks', recursive=True) +\
                  glob.glob('product/**/test/**/mocks', recursive=True)

EXCLUDE_DIRECTORIES = [
    '.git',
    'build',
    'contrib',
    'product/rcar/src/CMSIS-FreeRTOS',
    'unit_test/unity_mocks',
] + UNIT_TEST_MOCKS

#
# File types to check
#
FILE_TYPES = [
    '*.c',
    '*.h',
    '*.py',
    'CMakeLists.txt',
]


def convert_tabs_to_spaces(path):
    print(f'\tConverting all tabs in {path} into spaces...')
    try:
        _, temp_file = tempfile.mkstemp(prefix='tabs_to_spaces_')
        print(f'Using {temp_file}')
        subprocess.check_call(f'expand -t4 {path} > {temp_file}',
                              shell=True)
        shutil.copyfile(temp_file, path)
    except Exception as e:
        print(f'Error: Failed to convert file {path} with {e}')
        sys.exit(1)
    finally:
        if os.path.exists(temp_file):
            os.remove(temp_file)


def check_files(file_paths, convert):
    tabs_found_count = 0

    for path in file_paths:
        try:
            with open(path, encoding='utf-8') as file:
                file_has_tabs = False
                for line, string in enumerate(file):
                    if '\t' in string:
                        print(f'{path}:{line + 1} has tab instead of spaces')
                        tabs_found_count += 1
                        file_has_tabs = True
                if convert and file_has_tabs:
                    convert_tabs_to_spaces(path)
        except UnicodeDecodeError:
            print(f'Invalid file format {path}')
    return tabs_found_count


def run(convert=False):
    print(banner('Checking the presence of tabs in the code...'))

    if convert:
        print('Conversion mode is enabled.')

    files = get_filtered_files(EXCLUDE_DIRECTORIES, FILE_TYPES)
    tabs_found_count = check_files(files, convert)
    if tabs_found_count == 0:
        print('No tabs found')
        return True

    print(f'{tabs_found_count} tab(s) found.')
    return False


def parse_args(argv, prog_name):
    parser = argparse.ArgumentParser(
        prog=prog_name,
        description='Perform checks for presence of tabs in the code')

    parser.add_argument('-c', '--convert', dest='convert',
                        required=False, default=False, action='store_true',
                        help='Convert tabs to 4 spaces.')

    return parser.parse_args(argv)


def main(argv=[], prog_name=''):
    args = parse_args(argv, prog_name)
    return 0 if run(args.convert) else 1


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:], sys.argv[0]))
