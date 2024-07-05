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
Filters out cmock and cmsis from coverage report so that only actual SCP
code is evaluated. A report can have their own filter by appending
the 'regex_list'.
"""

default_regex = [re.compile(r"contrib/cmock/"),
                 re.compile(r"contrib/cmsis/")]

# Append specific files filter.
regex_list = {
    'scp_v2_fwk_test_coverage.info': [re.compile(r"framework/test/")],
    'scp_v2_unit_test_coverage.info': [re.compile(r"contrib/cmock/"),
                                       re.compile(r"module/.*/test"),
                                       re.compile(r"unit_test/")],
    'scp_v2_product_test_coverage.info': [re.compile(r"contrib/cmock/"),
                                          re.compile(r"framework/"),
                                          re.compile(r"product/.*/test"),
                                          re.compile(r"unit_test/")],
}

STARTER = "SF:"
ENDED = "end_of_record"


def get_filtered_info(filename):
    within = False
    filter = False
    dump_lines = []
    regexes = regex_list.get(filename, default_regex)
    out_filename = filename.replace('.info', '_filtered.info')

    with open(out_filename, "w") as fd:
        with open(filename, "r") as fd_in:
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
    return True


def parse_args(argv, prog_name):
    parser = argparse.ArgumentParser(
        prog=prog_name,
        description='Filters out cmock and cmsis from coverage report\n'
                    'To filter specific regex for a report, please append '
                    'the regex list in this script.',
        formatter_class=RawDescriptionHelpFormatter)

    parser.add_argument('-f', '--filename', dest='filename', required=True,
                        type=str, action='store', help=f'Required argument.\
                        Filename of the coverage report to be filtered.')

    return parser.parse_args(argv)


def main(argv=[], prog_name=''):
    args = parse_args(argv, prog_name)
    return 0 if get_filtered_info(args.filename) else 1


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:], sys.argv[0]))
