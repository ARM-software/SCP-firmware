#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

"""
Check whether the files adhere to the prescribed coding style. Validation
is performed by checkpatch.pl which is found on the environment path or
via user supplied path.
"""

import argparse
import os
import fnmatch
import sys
import subprocess

#
# Checkpatch.pl location (assume it is available through the environment path
# by default)
#
script_path = 'checkpatch.pl'

#
# Directories to scan. Only used when --input-mode is set to "project".
#
DIRECTORIES = [
    'arch',
    'framework',
    'module',
    'product',
    'tools',
]

#
# Supported file types. Only used when --input-mode is set to "project".
#
FILE_TYPES = [
    '*.c',
    '*.h',
]

#
# Default ignored types. These are rules within checkpatch that conflict with
# the SCP/MCP Software coding style and so they should never be enabled.
#
IGNORED_TYPES = [
    'LEADING_SPACE',  # Incompatible with spaces for indentation
    'CODE_INDENT',  # Incompatible with spaces for indentation
    'SUSPECT_CODE_INDENT',  # Incompatible with spaces for indentation
    'POINTER_LOCATION',  # Doesn't agree with our function declaration style
    'BLOCK_COMMENT_STYLE',  # Doesn't tolerate asterisks on each block line
    'AVOID_EXTERNS',  # We use the extern keyword
    'NEW_TYPEDEFS',  # We add new typedefs
    'VOLATILE',  # We use volatile
    'MACRO_WITH_FLOW_CONTROL',  # Some 'capture' macros use do/while loops
    'LINE_SPACING',  # We don't require a blank line after declarations
    'SPLIT_STRING',  # We allow strings to be split across lines
    'FILE_PATH_CHANGES',  # Specific to the kernel development process
    'PREFER_PACKED',  # __packed is not available in Arm Compiler 6
]

error_count = 0


def is_valid_file_type(filename):
    return any([fnmatch.fnmatch(filename, t) for t in FILE_TYPES])


def check_file(checkpatch_params, filename):
    global error_count

    cmd = '{} {}'.format(checkpatch_params, filename)

    try:
        subprocess.check_call(cmd, shell=True, stdin=0)
    except subprocess.CalledProcessError:
        error_count += 1


def main(argv=[], prog_name=''):
    global script_path
    print('Arm SCP/MCP Software Checkpatch Wrapper')
    parser = argparse.ArgumentParser(prog=prog_name)

    input_mode_list = ['stdin', 'project']

    # Optional parameters
    parser.add_argument('-s', '--spacing', action='store_true',
                        help='Check for correct use of spaces',
                        required=False)

    parser.add_argument('-l', '--line-length', action='store_true',
                        dest='length',
                        help='Check for lines longer than 80 characters',
                        required=False)

    parser.add_argument('-i', '--initializers', action='store_true',
                        help='Check for redundant variable initialization',
                        required=False)

    parser.add_argument('-m', '--input-mode', choices=input_mode_list,
                        help='Input mode for the content to be checked. '
                             'Default: %(default)s',
                        required=False, default=input_mode_list[0])

    parser.add_argument('-p', '--path', action='store', dest='path',
                        help='Path to checkpatch.pl file. If not specified, '
                             'the script will be found on the environment '
                             'path.',
                        required=False)

    args = parser.parse_args(argv)

    # Override path to checkpatch.pl if necessary
    if args.path:
        script_path = args.path

    # Print the path to checkpatch.pl as confirmation
    print('checkpatch.pl path:', script_path, '\n')

    # Enable optional tests
    if not args.spacing:
        IGNORED_TYPES.extend(['SPACING', 'MISSING_SPACE', 'BRACKET_SPACE'])

    if not args.length:
        IGNORED_TYPES.extend(['LONG_LINE', 'LONG_LINE_COMMENT',
                              'LONG_LINE_STRING'])
    if not args.initializers:
        IGNORED_TYPES.extend(['GLOBAL_INITIALISERS', 'INITIALISED_STATIC'])

    ignore_list = '--ignore ' + (','.join(map(str, IGNORED_TYPES)))

    checkpatch_params = '{} --show-types --no-tree --no-summary {}'.format(
        script_path,
        ignore_list,
    )

    if args.input_mode == 'project':
        print("Checking the coding style of the whole project...")
        checkpatch_params += ' --terse --file'
        for directory in DIRECTORIES:
            for root, dirs, files in os.walk(directory):
                for file in files:
                    filename = os.path.join(root, file)
                    if is_valid_file_type(file):
                        check_file(checkpatch_params, filename)
        if error_count > 0:
            print('{} files contained coding style errors.'.
                  format(error_count))

    elif args.input_mode == 'stdin':
        print("Checking content via standard input...")
        check_file(checkpatch_params, '-')

    else:
        print('FAILED: Invalid input mode')
        return 1

    if error_count > 0:
        print('FAILED: One or more files contained coding style errors.')
        return 1

    print('PASSED: No files contained coding style errors.')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:], sys.argv[0]))
