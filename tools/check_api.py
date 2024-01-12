#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2019-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

"""
    Check for usage of banned API (banned_api.lst).
"""

import argparse
import glob
import sys
from utils import banner, get_filtered_files

#
# Default banned list file
#
BANNED_LIST_DEFAULT = './tools/banned_api.lst'


# Exclude all mod_test 'mocks' directories
UNIT_TEST_MOCKS = glob.glob('module/**/test/**/mocks', recursive=True)

#
# Directories to exclude
#
EXCLUDE_DIRECTORIES = [
    '.git',
    'build',
    'tools',
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
]


def read_banned_api_file(banned_api_file):
    banned_api = list()
    with open(banned_api_file) as file:
        for fname in file:
            if fname[0] == '#':
                continue
            banned_api.append(fname.rstrip())

        print('\tBanned API list: {}'.format(banned_api))

    return banned_api


def check_file(path, banned_apis):
    file_error = False
    try:
        with open(path, encoding='utf-8') as file:
            for line, string in enumerate(file):
                for fname in banned_apis:
                    if fname in string:
                        file_error = True
                        print(f'{path}:{line + 1}:{fname}')
    except UnicodeDecodeError:
        print(f'Invalid file format {path}')
    return file_error


def check_files(file_paths, banned_apis):
    file_error = False

    for path in file_paths:
        file_error = check_file(path, banned_apis) or file_error

    if not file_error:
        print('No banned API found')
        return True
    print('Banned API found')
    return False


def run(banned_api_list=BANNED_LIST_DEFAULT):
    print(banner('Checking for usage of banned APIs...'))
    banned_api = read_banned_api_file(banned_api_list)
    files = get_filtered_files(EXCLUDE_DIRECTORIES, FILE_TYPES)

    return check_files(files, banned_api)


def parse_args(argv, prog_name):
    parser = argparse.ArgumentParser(
        prog=prog_name,
        description='Perform basic API checks to SCP-Firmware')

    parser.add_argument('-i', '--banned-api-list', dest='banned_api_list',
                        required=False, default=BANNED_LIST_DEFAULT, type=str,
                        action='store', help=f'Banned API file location, if \
                        banned API list is not given, the default location \
                        is {BANNED_LIST_DEFAULT}')

    return parser.parse_args(argv)


def main(argv=[], prog_name=''):
    args = parse_args(argv, prog_name)
    return 0 if run(args.banned_api_list) else 1


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:], sys.argv[0]))
