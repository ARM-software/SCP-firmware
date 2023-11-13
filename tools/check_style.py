#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

"""
    Check for code style violations.
"""

import argparse
import subprocess
import sys
from utils import banner, get_previous_commit

#
# Default output file
#
DEFAULT_OUTPUT_FILE = 'code-style.patch'


def run(output_file=DEFAULT_OUTPUT_FILE, commit_hash=get_previous_commit()):
    print(banner(f'Run coding style checks against {commit_hash[:8]}'))

    # Run git clang-format with the previous commit hash and capture the patch
    result = subprocess.run(
        ['git', 'clang-format', '--quiet', '--diff', commit_hash],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    patch = result.stdout

    if patch and not patch.startswith('no modified files to format'):
        # Write the patch to code-style.patch
        with open(output_file, 'w') as patch_file:
            patch_file.write(patch)

        # Print messages
        print('Code style deviations were identified.')
        print('')
        print('Please apply the supplied patch with:')
        print(f'    patch -p1 < {output_file}')
        return False

    print('No code style deviations were identified.')
    print('')
    return True


def parse_args(argv, prog_name):
    parser = argparse.ArgumentParser(
        prog=prog_name,
        description='Perform code style check to SCP-Firmware')

    parser.add_argument('-o', '--output-file', dest='output_file',
                        required=False, default=DEFAULT_OUTPUT_FILE, type=str,
                        action='store', help=f'Output file location, if it is\
                        not specified, the default value will be\
                        {DEFAULT_OUTPUT_FILE}')

    parser.add_argument('-c', '--commit', dest='commit_hash',
                        required=False, default=get_previous_commit(),
                        type=str, action='store', help='Specify a commit \
                        hash. If not specified, defaults to the previous \
                        commit.')

    return parser.parse_args(argv)


def main(argv=[], prog_name=''):
    args = parse_args(argv, prog_name)
    return 0 if run(args.output_file, args.commit_hash) else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:], sys.argv[0]))
