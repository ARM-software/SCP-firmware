#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

"""
Check if a given file includes the correct license header.
This checker supports the following comment styles:
    * Used by .c, .h and .s/.S (GCC) files
    ; Used by .s (ARM toolchain) and .scat (scatter file) files
    # Used by Makefile (including .mk), .py (Python) and dxy (Doxygen) files
"""

import argparse
import os
import fnmatch
import re
import sys
import datetime
import subprocess
import glob
from dataclasses import dataclass, asdict
from itertools import islice
from utils import banner, get_filtered_files, get_previous_commit

#
# Directories to exclude
#

# Exclude all mod_test 'mocks' directories
UNIT_TEST_MOCKS = glob.glob('module/**/test/**/mocks', recursive=True)

EXCLUDE_DIRECTORIES = [
    '.git',
    'build',
    'contrib',
    'product/rcar/src/CMSIS-FreeRTOS',
    'unit_test/unity_mocks',
] + UNIT_TEST_MOCKS

#
# Supported file types
#
FILE_TYPES = [
    'Makefile',
    '*.mk',
    '*.c',
    '*.h',
    '*.s',
    '*.S',
    '*.py',
    '*.scat',
    '*CMakeLists.txt',
    '*.cmake',
    "*.rb",
    "*.yaml",
    "*.yml",
]

#
# Supported comment styles (Python regex)
#
COMMENT_PATTERN = '^(( \\*)|(;)|(\\#))'
COMPANY_PATTERN = '(Arm|'\
                  'Renesas|'\
                  'Linaro)'
COMPANY_FULL_NAME_PATTERN = \
    '(Arm Limited and Contributors|'\
    'Renesas Electronics Corporation|'\
    'Linaro Limited and Contributors)'

#
# License pattern to match
#
LICENSE_PATTERN = \
    '{0} {1} SCP/MCP Software$\n'\
    '({0} Copyright \\(c\\) (?P<years>[0-9]{{4}}(-[0-9]{{4}})?), {2}.'\
    ' All rights(( )|(\n{0} ))reserved.$\n)+'\
    '{0}$\n'\
    '{0} SPDX-License-Identifier: BSD-3-Clause$\n'\
    .format(COMMENT_PATTERN, COMPANY_PATTERN, COMPANY_FULL_NAME_PATTERN)

#
# The number of lines from the beginning of the file to search for the
# copyright header. This limit avoids the tool searching the whole file when
# the header always appears near the top.
#
# Note: The copyright notice does not usually start on the first line of the
# file. The value should be enough to include the all of the lines in the
# LICENSE_PATTERN, plus any extra lines that appears before the license. The
# performance of the tool may degrade if this value is increased significantly.
#
HEAD_LINE_COUNT = 10


@dataclass
class Analysis:
    error_copyright: int = 0
    error_year_format: int = 0
    error_current_year: int = 0

    def has_errors(self):
        return any([value for value in asdict(self).values()])

    def append(self, partial_analysis):
        for elem, count in asdict(partial_analysis).items():
            if hasattr(self, elem):
                setattr(self, elem, getattr(self, elem) + count)
            else:
                # Handle the error or ignore
                print(f'Warning: Attribute {elem} not found.')
        return self

    def __str__(self):
        msg = ''
        msg += self._str_message('files with invalid copyright',
                                 self.error_copyright)
        msg += self._str_message('files with invalid year(s) format',
                                 self.error_year_format)
        msg += self._str_message('files with incorrect year ranges',
                                 self.error_current_year)
        return msg

    def _str_message(self, name, count) -> str:
        return f'- {count} {name}.\n'


def get_changed_files(commit_range):
    """
    Get a list of changed files for the given commit range.
    """
    #
    # git command using diff-filter to include Added (A), Copied (C),
    # Modified (M), Renamed (R), type changed (T), Unmerged (U), Unknown (X)
    # files Deleted files (D) are not included
    #
    diff_cmd = f'git diff --name-status {commit_range} --diff-filter=ACMRTUX'

    try:
        result = subprocess.Popen(
            diff_cmd,
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)
    except subprocess.CalledProcessError as e:
        print('ERROR ' + e.returncode + ': Failed to get changed files')
        return None

    result_out, _ = result.communicate()
    result_out = result_out.decode('utf-8').strip().split('\n')
    result_out = list(map(lambda diff: diff.split('\t'), result_out))

    # Remove files renamed with 100% similarity. R100 denotes this.
    result_out = list(filter(lambda diff: diff[0] != 'R100', result_out))

    # Get modified file and convert to absolute path
    current_directory = os.getcwd()
    return list(map(
        lambda diff: os.path.abspath(os.path.join(current_directory,
                                                  diff[-1])), result_out))


def check_copyright(filename, pattern, analysis):
    with open(filename, encoding='utf-8') as file:
        # Read just the first HEAD_LINE_COUNT lines of a file
        head = ''.join(islice(file, HEAD_LINE_COUNT))

        match = pattern.search(head)
        if not match:
            print(f'{filename}: Invalid copyright header.')
            analysis.error_copyright += 1
            return

        years = match.group('years').split('-')
        if len(years) > 1:
            if years[0] > years[1]:
                print(f'{filename}: Invalid year format.')
                analysis.eror_year_format += 1
                return

        now = datetime.datetime.now()
        final_year = len(years) - 1
        if int(years[final_year]) != now.year:
            print(f'{filename}: Outdated copyright year range.')
            analysis.error_current_year += 1
            return


def run(commit_hash=get_previous_commit()):
    print(banner('Checking the copyrights in the code...'))

    changed_files = get_changed_files(commit_hash)
    if len(changed_files) == 0:
        return False

    filtered_files = get_filtered_files(EXCLUDE_DIRECTORIES, FILE_TYPES)
    changed_files = list(set(changed_files) & set(filtered_files))

    pattern = re.compile(LICENSE_PATTERN, re.MULTILINE)
    analysis = Analysis()

    for file in changed_files:
        partial_analysis = Analysis()
        check_copyright(file, pattern, partial_analysis)
        analysis.append(partial_analysis)

    print(analysis)
    return not analysis.has_errors()


def parse_args(argv, prog_name):
    parser = argparse.ArgumentParser(
        prog=prog_name,
        description='Perform code style check the correct license header')

    parser.add_argument('-c', '--commit', dest='commit_hash',
                        required=False, default=get_previous_commit(),
                        type=str, action='store', help='Specify a commit \
                        hash. If not specified, defaults to the previous \
                        commit.')

    return parser.parse_args(argv)


def main(argv=[], prog_name=''):
    args = parse_args(argv, prog_name)
    return 0 if run(args.commit_hash) else 1


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:], sys.argv[0]))
