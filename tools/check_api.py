#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

"""
    Check for usage of banned API (banned_api.lst).
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
import fnmatch

BANNED_LIST = './tools/banned_api.lst'
BANNED_API = list()

#
# Directories to exclude
#
EXCLUDE_DIRECTORIES = [
    '.git',
    'build',
    'tools',
    'contrib/cmsis/git',
    "contrib/run-clang-format/git",
    'product/rcar/src/CMSIS-FreeRTOS',
]

#
# Exclude patterns (applied to files only)
#
EXCLUDE = [
    '*.html',
    '*.xml',
    '*.css',
    '*.gif',
    '*.dat',
    '*.pyc',
    '*.jar',
    '*.md',
    '*.swp',
    '*.a',
    '*.pdf',
]

#
# File types to check
#
FILE_TYPES = [
    '*.c',
    '*.h',
]


def is_valid_type(filename):
    for file_type in FILE_TYPES:
        if fnmatch.fnmatch(filename, file_type):
            return True

    return False


def main(argv=[], prog_name=''):
    print('Checking for usage of banned APIs...')

    illegal_use = 0

    with open(BANNED_LIST) as file:
        for fname in file:
            if fname[0] == '#':
                continue
            BANNED_API.append(fname.rstrip())

        print("\tBanned API list: {}".format(BANNED_API))

    cwd = os.getcwd()

    for i, directory in enumerate(EXCLUDE_DIRECTORIES):
        EXCLUDE_DIRECTORIES[i] = os.path.abspath(directory)
        print("\tAdding to the exclude list: {}"
              .format(EXCLUDE_DIRECTORIES[i]))

    for root, dirs, files in os.walk(cwd, topdown=True):
        #
        # Exclude directories based on the EXCLUDE_DIRECTORIES pattern list
        #
        dirs[:] = [d for d in dirs
                   if os.path.join(root, d) not in EXCLUDE_DIRECTORIES]

        #
        # Exclude files based on the EXCLUDE pattern list
        #
        matches = list()
        for filename in files:
            for file_pattern in EXCLUDE:
                if fnmatch.fnmatch(filename, file_pattern):
                    matches.append(filename)
                    break

        for match in matches:
            files.remove(match)

        #
        # Check files
        #
        for filename in files:
            path = os.path.join(root, filename)
            content = ''
            with open(path, encoding="utf-8") as file:
                for line, string in enumerate(file):
                    for fname in BANNED_API:
                        if fname in string:
                            print("{}:{}:{}".format(path, line + 1, fname),
                                  file=sys.stderr)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:], sys.argv[0]))
