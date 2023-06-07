#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
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
import os
import fnmatch
import re
import sys
import datetime
import subprocess
import glob
from itertools import islice

#
# Directories to exclude
#

# Exclude all mod_test "mocks" directories
UNIT_TEST_MOCKS = glob.glob('module/**/test/**/mocks', recursive=True)

EXCLUDE_DIRECTORIES = [
    '.git',
    'build',
    'contrib/cmsis/git',
    "contrib/run-clang-format/git",
    "contrib/cmock/git",
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
COMPANY_PATTERN = '(Arm|Renesas|Linaro)'
COMPANY_FULL_NAME_PATTERN = \
    '(Arm Limited and Contributors|Renesas Electronics Corporation|'\
    'Linaro Limited and Contributors)'

#
# git command using diff-filter to include Added (A), Copied (C), Modified (M),
# Renamed (R), type changed (T), Unmerged (U), Unknown (X) files
# Deleted files (D) are not included
#
GIT_CMD = \
    'git diff-tree --name-only --no-commit-id -r --diff-filter=ACMRTUX HEAD'

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


class ErrorYear(Exception):
    pass


class ErrorCopyright(Exception):
    pass


class ErrorYearNotCurrent(Exception):
    pass


def is_valid_directory(filename):
    for dir in EXCLUDE_DIRECTORIES:
        if filename.startswith(dir):
            return False
    return True


def is_valid_file_type(filename):
    for file_type in FILE_TYPES:
        if fnmatch.fnmatch(filename, file_type):
            return True
    return False


def check_copyright(pattern, filename):
    with open(filename, encoding="utf-8") as file:
        # Read just the first HEAD_LINE_COUNT lines of a file
        head_lines = islice(file, HEAD_LINE_COUNT)
        head = ''
        for line in head_lines:
            head += line

        match = pattern.search(head)
        if not match:
            raise ErrorCopyright

        years = match.group('years').split('-')
        if len(years) > 1:
            if years[0] > years[1]:
                raise ErrorYear

        now = datetime.datetime.now()
        final_year = len(years) - 1
        if int(years[final_year]) != now.year:
            raise ErrorYearNotCurrent


def main():
    pattern = re.compile(LICENSE_PATTERN, re.MULTILINE)
    error_year_count = 0
    error_copyright_count = 0
    error_incorrect_year_count = 0

    print("Checking the copyrights in the code...")

    cwd = os.getcwd()
    print("Executing from {}".format(cwd))

    try:
        result = subprocess.Popen(
            GIT_CMD,
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)
    except subprocess.CalledProcessError as e:
        print("ERROR " + e.returncode + ": Failed to get last changed files")
        return 1

    for line in result.stdout:
        filename = line.decode("utf-8").strip('\n')

        if is_valid_file_type(filename) and is_valid_directory(filename):
            try:
                check_copyright(pattern, filename)
            except ErrorYear:
                print("{}: Invalid year format.".format(filename))
                error_year_count += 1

            except ErrorCopyright:
                print("{}: Invalid copyright header.".format(filename))
                error_copyright_count += 1

            except ErrorYearNotCurrent:
                print("{}: Outdated copyright year range.".
                      format(filename))
                error_incorrect_year_count += 1

    if error_year_count != 0 or error_copyright_count != 0 or \
       error_incorrect_year_count != 0:
        print("\t{} files with invalid year(s) format."
              .format(error_year_count))
        print("\t{} files with invalid copyright."
              .format(error_copyright_count))
        print("\t{} files with incorrect year ranges."
              .format(error_incorrect_year_count))

        return 1
    else:
        print("Check copyright - No errors found.")
        return 0


if __name__ == "__main__":
    sys.exit(main())
