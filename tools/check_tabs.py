#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
"""
    Check for tabs in the source code.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
import fnmatch
import glob

#
# Directories to exclude
#

# Exclude all mod_test "mocks" directories
UNIT_TEST_MOCKS = glob.glob('module/**/test/**/mocks', recursive=True)
UNIT_TEST_EXT = glob.glob('module/**/test/ext', recursive=True)

EXCLUDE_DIRECTORIES = [
    '.git',
    'build',
    'contrib/cmsis/git',
    "contrib/run-clang-format/git",
    "contrib/cmock/git",
    'product/rcar/src/CMSIS-FreeRTOS',
    'unit_test/unity_mocks',
] + UNIT_TEST_MOCKS + UNIT_TEST_EXT

#
# Exclude patterns (applied to files only)
#
EXCLUDE = [
    "Makefile",
    "*.mk",
    "*.html",
    "*.xml",
    "*.css",
    "*.gif",
    "*.dat",
    "*.swp",
    "*.pyc",
    ".gitmodules",
    "*.svg",
    "*.a",
    "*.pdf",
    "Makefile.*"
]


def convert(path):
    print("\tConverting all tabs in %s into spaces..." % path)
    try:
        file, temp_file = tempfile.mkstemp(prefix='tabs_to_spaces_')
        print("Using %s" % temp_file)
        subprocess.check_call('expand -t4 %s > %s' % (path, temp_file),
                              shell=True)
        shutil.copyfile(temp_file, path)
    except Exception as e:
        print("Error: Failed to convert file %s with %s" % (path, e))
        sys.exit(1)
    finally:
        if os.path.exists(temp_file):
            os.remove(temp_file)


def main(argv=[], prog_name=''):
    parser = argparse.ArgumentParser(prog=prog_name)
    parser.add_argument('-c', '--convert',
                        help='Convert tabs to 4 spaces.',
                        action='store_true',
                        default=False)
    args = parser.parse_args(argv)

    print('Checking the presence of tabs in the code...')
    if args.convert:
        print("Conversion mode is enabled.")

    tabs_found_count = 0

    # Get the files ignored by Git
    # (This is better than 'git check-ignore' because it includes the files
    #  excluded by .git/info/exclude)
    git_clean_output = subprocess.check_output("git clean -ndX".split())
    git_clean_output = git_clean_output.decode()
    git_ignores = [line.split()[-1] for line in git_clean_output.splitlines()]

    cwd = os.getcwd()
    print("Executing from %s" % cwd)

    for i, directory in enumerate(EXCLUDE_DIRECTORIES):
        EXCLUDE_DIRECTORIES[i] = os.path.abspath(directory)
        print("\tAdding to the exclude list: %s" % EXCLUDE_DIRECTORIES[i])

    for root, dirs, files in os.walk(cwd, topdown=True):
        #
        # Exclude directories based on the EXCLUDE_DIRECTORIES pattern list
        #
        dirs[:] = [d for d in dirs
                   if os.path.join(root, d) not in EXCLUDE_DIRECTORIES]

        #
        # Exclude files based on the EXCLUDE pattern list and the files
        # Git ignores.
        #
        matches = list()

        files = [f for f in files
                 if os.path.join(root, f) not in git_ignores]

        for filename in files:
            for file_pattern in (EXCLUDE + git_ignores):
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
            print("processing %s" % filename)
            with open(path, encoding="utf-8") as file:
                for line, string in enumerate(file):
                    if '\t' in string:
                        print('%d:%s has tab' % (line, path))
                        tabs_found_count += 1
                        if args.convert:
                            convert(path)
                            break

    if tabs_found_count == 0:
        print("No tabs found")
        return 0
    else:
        print('%d tab(s) found.' % tabs_found_count)
        return 1


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:], sys.argv[0]))
