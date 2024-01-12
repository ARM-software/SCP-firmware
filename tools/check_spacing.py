#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

"""
    Check for trailing spaces and non-UNIX line endings in the source code.
"""

import argparse
import os
import re
import sys
import fnmatch
import glob
from dataclasses import dataclass, asdict
from utils import banner, get_filtered_files


#
# Directories to exclude
#

# Exclude all mod_test 'mocks' directories
UNIT_TEST_MOCKS = glob.glob('module/**/test/**/mocks', recursive=True)

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
    '*.cmake',
    'CMakeLists.txt',
]

#
# Code file types to check
#
CODE_FILES = [
    '*.c',
    '*.h',
]

#
# Code keywords to check
#
KEYWORDS = [
    'for',
    'if',
    'switch',
    'while',
]


@dataclass
class Analysis:
    trailing_spaces: int = 0
    trailing_lines: int = 0
    incorrect_spaces: int = 0
    modified_files: int = 0
    non_unix_eol_files: int = 0
    missing_new_line_files: int = 0

    def has_errors(self) -> bool:
        return any([value for value in asdict(self).values()])

    def add(self, partial_analysis):
        for elem, count in asdict(partial_analysis).items():
            if hasattr(self, elem):
                setattr(self, elem, getattr(self, elem) + count)
            else:
                # Handle the error or ignore
                print(f'Warning: Attribute {elem} not found.')
        return self

    def __str__(self) -> str:
        msg = ''
        msg += self._str_message('trailing spaces', self.trailing_spaces)
        msg += self._str_message('trailing lines', self.trailing_lines)
        msg += self._str_message('abnormal spaces', self.incorrect_spaces)
        msg += self._str_message('files with non-UNIX or mixed line endings',
                                 self.non_unix_eol_files)
        msg += self._str_message('files with missing newlines at EOF',
                                 self.missing_new_line_files)
        if self.modified_files:
            msg += f'- {self.modified_files} files modified.\n'
        return msg

    def _str_message(self, name, count) -> str:
        if count == 0:
            return f'- No {name} found.\n'
        else:
            return f'- {count} {name} found.\n'


def is_valid_code_type(filename):
    return any(fnmatch.fnmatch(filename, file_type)
               for file_type in CODE_FILES)


def get_regex_patterns(keywords):
    regex_patterns = dict.fromkeys(keywords, 0)
    for keyword in keywords:
        regex_patterns[keyword] = re.compile(
            f'(.*\\W)({keyword})(\\s*)(\\(.*)')

    return regex_patterns


def check_line(path, line, regex_patterns, analysis, trim, correct):
    # Note that all newlines are converted to '\n',
    # so the following will work regardless of
    # what the underlying file format is using to
    # represent a line break.
    if line.endswith(' \n'):
        print('{}:{} has trailing space'.format(line, path))
        analysis.trailing_spaces += 1
        if trim:
            line = line.rstrip()+'\n'
    if not is_valid_code_type(os.path.basename(path)):
        return line

    for keyword, regex_pattern in regex_patterns.items():
        if line.find(keyword) < 0:
            continue
        m = regex_pattern.search(line)
        if m and m.group(3) != ' ':
            analysis.incorrect_spaces += 1
            print(f'Abnormal spacing. \'{keyword}\', {path}:{line} \
                --> {line.rstrip()}')
            if correct:
                line = m.group(1) + m.group(2) + ' ' + \
                            m.group(4) + '\n'

    return line


def write_file(path, analysis, modified_file, trim, correct):
    #
    # Trim and/or correct file, depending on the provided arguments
    #
    write_file = False
    if trim and (analysis.trailing_spaces
                 or analysis.trailing_lines) != 0:
        print('Trimming {}...'.format(path))
        write_file = True
    if correct and analysis.incorrect_spaces != 0:
        print('Correcting {}...'.format(path))
        write_file = True
    if write_file:
        analysis.modified_files += 1
        with open(path, 'w') as file:
            file.write(modified_file)


def check_files(file_paths, regex_patterns, trim, correct):
    analysis = Analysis()
    for path in file_paths:
        partial_analysis = Analysis()
        content = ''
        try:
            with open(path, encoding='utf-8') as file:
                lines = file.readlines()
                if lines and not lines[-1].endswith('\n'):
                    partial_analysis.missing_new_line_files += 1
                    print(f'{path}: is missing a new line at the end of file')

                for line in lines:
                    processed_line = check_line(path,
                                                line,
                                                regex_patterns,
                                                partial_analysis,
                                                trim,
                                                correct)
                    content += processed_line

        except UnicodeDecodeError:
            print(f'Invalid file format {path}')

        if content.endswith('\n\n'):
            print(f'Blank line at the end of file --> {path}')
            if trim:
                content = content.rstrip() + '\n'
            trailing_lines = len(content) - len(content.rstrip() + '\n')
            partial_analysis.trailing_lines += trailing_lines

        #
        # If file.newlines has been set it is either a string with
        # the determined line ending or a tuple with all the line
        # endings we have encountered
        #
        if file.newlines:
            if isinstance(file.newlines, tuple):
                print('{} has mixed line endings'.format(path))
                partial_analysis.non_unix_eol_files += 1
            elif file.newlines != '\n':
                print('{} has non-UNIX line endings'.format(path))
                partial_analysis.non_unix_eol_files += 1

        write_file(path, partial_analysis, content, trim, correct)

        analysis.add(partial_analysis)

    return analysis


def run(trim=False, correct=False):
    print(banner('Checking for incorrect spacing in the code...'))

    if trim:
        print('Trim mode is enabled.')
    if correct:
        print('Correct mode is enabled.')

    regex_patterns = get_regex_patterns(KEYWORDS)
    files = get_filtered_files(EXCLUDE_DIRECTORIES, FILE_TYPES)
    analysis = check_files(files, regex_patterns, trim, correct)
    print(analysis)

    return not analysis.has_errors()


def parse_args(argv, prog_name):
    parser = argparse.ArgumentParser(
        prog=prog_name,
        description='Perform checks for incorrect spacing in the code')

    parser.add_argument('-t', '--trim', dest='trim',
                        required=False, default=False, action='store_true',
                        help='Remove trailing spaces.')

    parser.add_argument('-c', '--correct', dest='correct',
                        required=False, default=False, action='store_true',
                        help='Correct spaces after keywords.')

    return parser.parse_args(argv)


def main(argv=[], prog_name=''):
    args = parse_args(argv, prog_name)
    return 0 if run(args.trim, args.correct) else 1


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:], sys.argv[0]))
