#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
"""
    Check for trailing spaces and non-UNIX line endings in the source code.
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

EXCLUDE_DIRECTORIES = [
    '.git',
    'build',
    'tools',
    'contrib/cmsis/git',
    "contrib/run-clang-format/git",
    "contrib/cmock/git",
    'product/rcar/src/CMSIS-FreeRTOS',
    'unit_test/unity_mocks',
] + UNIT_TEST_MOCKS

#
# Exclude patterns (applied to files only)
#
EXCLUDE = [
    "*.html",
    "*.xml",
    "*.css",
    "*.gif",
    "*.dat",
    "*.pyc",
    "*.jar",
    "*.md",
    "*.swp",
    "*.a",
    "*.pdf",
    ".*"
]

KEYWORDS = [
    'for',
    'if',
    'switch',
    'while',
]

#
# File types for which spaces after keywords will be corrected
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
    parser = argparse.ArgumentParser(prog=prog_name)
    parser.add_argument('-t', '--trim',
                        help='Remove trailing spaces.',
                        action='store_true',
                        default=False)
    parser.add_argument('-c', '--correct',
                        help='Correct spaces after keywords.',
                        action='store_true',
                        default=False)
    args = parser.parse_args(argv)

    print('Checking for incorrect spacing in the code...')
    if args.trim:
        print("Trim mode is enabled.")
    if args.correct:
        print("Correct mode is enabled.")

    regex_patterns = dict.fromkeys(KEYWORDS, 0)

    trailing_spaces_count = 0
    trailing_lines_count = 0
    correct_spaces_count = 0
    modified_files = 0
    non_unix_eol_files = 0
    missing_new_lines_files = 0

    cwd = os.getcwd()
    print("Executing from {}".format(cwd))

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

        for keyword in KEYWORDS:
            regex_patterns[keyword] = re.compile(
                '(.*\\W)(%s)(\\s*)(\\(.*)' % keyword)
        #
        # Check files
        #
        for filename in files:
            path = os.path.join(root, filename)
            content = ''
            trailing_spaces = 0
            trailing_lines = 0
            incorrect_spaces = 0

            with open(path, encoding="utf-8") as file:
                lines = file.readlines()
                last_line_number = len(lines)-1
                for line, string in enumerate(lines):
                    if line == last_line_number and string[-1] != '\n':
                        print('{} is missing a new line at the end of file'.
                              format(path))
                        missing_new_lines_files += 1

                    # Note that all newlines are converted to '\n',
                    # so the following will work regardless of
                    # what the underlying file format is using to
                    # represent a line break.
                    if string.endswith(' \n'):
                        print('{}:{} has trailing space'.format(line, path))
                        trailing_spaces += 1
                        if args.trim:
                            string = string.rstrip()+'\n'
                    if not is_valid_type(os.path.basename(path)):
                        content += string
                        continue
                    for keyword in KEYWORDS:
                        key_index = string.find(keyword)
                        if key_index == -1:
                            continue
                        m = regex_patterns[keyword].search(string)
                        if m and m.group(3) != ' ':
                            incorrect_spaces += 1
                            print('Abnormal spacing. "{}", {}:{} --> {}'
                                  .format(keyword, path, line,
                                          string.rstrip()))
                            if args.correct:
                                string = m.group(1) + m.group(2) + ' ' + \
                                         m.group(4) + '\n'
                    content += string

                if content.endswith('\n\n'):
                    print('Blank line at the end of file --> {}'.format(path))
                    c_len = len(content)
                    if args.trim:
                        content = content.rstrip()+'\n'
                    trailing_lines += c_len - len(content.rstrip() + '\n')
                #
                # If file.newlines has been set it is either a string with
                # the determined line ending or a tuple with all the line
                # endings we have encountered
                #
                if file.newlines:
                    if isinstance(file.newlines, tuple):
                        print('{} has mixed line endings'.format(path))
                        non_unix_eol_files += 1
                    elif file.newlines != '\n':
                        print('{} has non-UNIX line endings'.format(path))
                        non_unix_eol_files += 1

            #
            # Trim and/or correct file, depending on the provided arguments
            #
            write_file = False
            if args.trim and (trailing_spaces or trailing_lines) != 0:
                print("Trimming {}...".format(path))
                write_file = True
            if args.correct and incorrect_spaces != 0:
                print("Correcting {}...".format(path))
                write_file = True
            if write_file:
                modified_files += 1
                with open(path, 'w') as file:
                    file.write(content)

            trailing_spaces_count += trailing_spaces
            trailing_lines_count += trailing_lines
            correct_spaces_count += incorrect_spaces

    if trailing_spaces_count == 0:
        print("No trailing spaces found")
    else:
        print('{} trailing spaces found.'.format(trailing_spaces_count))

    if trailing_lines_count == 0:
        print("No trailing lines found")
    else:
        print('{} trailing lines found.'.format(trailing_lines_count))

    if correct_spaces_count == 0:
        print("No abnormal spaces found")
    else:
        print('Abnormal spaces found on {} lines.'
              .format(correct_spaces_count))

    if (args.trim or args.correct) and modified_files:
        print("{} files modified".format(modified_files))

    if non_unix_eol_files == 0:
        print("No files with non-UNIX or mixed line endings found")
    else:
        print("{} files have non-UNIX or mixed line endings"
              .format(non_unix_eol_files))

    if missing_new_lines_files == 0:
        print("No files with missing newlines at EOF found")
    else:
        print("{} text files are missing newlines at EOF"
              .format(missing_new_lines_files))

    if (trailing_spaces_count or
            trailing_lines_count or
            correct_spaces_count or
            non_unix_eol_files or
            missing_new_lines_files):
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:], sys.argv[0]))
