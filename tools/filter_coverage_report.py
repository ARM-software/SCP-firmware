#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import re
import sys
import argparse
from argparse import RawDescriptionHelpFormatter
from collections import namedtuple

"""
Filters out test and mock code from coverage report so that
only actual SCP code is evaluated
"""

# Append new report filename here.
all_filename = [
    'scp_v2_fwk_test_coverage.info',
    'scp_v2_unit_test_coverage.info',
    'scp_v2_product_test_coverage.info',
]

# Append new report regex here.
regex_list = [
    [re.compile(r"framework/test/"),
     ],
    [re.compile(r"contrib/cmock/"),
     re.compile(r"module/.*/test"),
     re.compile(r"unit_test/"),
     ],
    [re.compile(r"contrib/cmock/"),
     re.compile(r"framework/"),
     re.compile(r"product/.*/test"),
     re.compile(r"unit_test/"),
     ],
]

STARTER = "SF:"
ENDED = "end_of_record"


def get_filtered_info(FileName):
    within = False
    filter = False
    dump_lines = []
    regexes = regex_list[all_filename.index(FileName)]
    OutFileName = FileName.replace('.info', '_filtered.info')

    with open(OutFileName, "w") as fd:
        with open(FileName, "r") as fd_in:
            for line in fd_in.readlines():

                if STARTER in line:
                    within = True

                    if any(regex.search(line) for regex in regexes):
                        filter = True

                if within or "TN:" in line:
                    dump_lines.append(line)

                if ENDED in line:
                    if not filter:
                        fd.writelines(dump_lines)

                    dump_lines = []
                    within = False
                    filter = False


def run(FileName):
    if FileName not in all_filename:
        print('ERROR: Filename not found. Please use supported filename:')
        print(*all_filename, sep='\n')
        return False

    get_filtered_info(FileName)
    return True


def parse_args(argv, prog_name):
    parser = argparse.ArgumentParser(
        prog=prog_name,
        description='Filters out test and mock code from coverage report\n'
                    'To add new coverage report, please append the report '
                    'filename and regex in this script.',
        formatter_class=RawDescriptionHelpFormatter)

    parser.add_argument('-f', '--filename', dest='FileName', required=True,
                        type=str, action='store', help=f'Required argument.\
                        Filename of the coverage report to be filtered. \
                        Supported filename: {all_filename}')

    return parser.parse_args(argv)


def main(argv=[], prog_name=''):
    args = parse_args(argv, prog_name)
    return 0 if run(args.FileName) else 1


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:], sys.argv[0]))
