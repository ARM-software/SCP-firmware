#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import subprocess
import fnmatch
import os
from typing import List, Tuple


def banner(text: str, columns: int = 80) -> str:
    """
    This function generates a banner by centering the provided text within a
    specified width (default is 80 columns) and padding it with asterisks (*)
    on both sides.

    Parameters:
    text (str): The text to be displayed in the banner.
    columns (int, optional): The total width of the banner including the text
    and asterisks. Defaults to 80.

    Returns:
    str: A string representing the formatted banner.
    """
    return f'\n\n{f" {text} ".center(columns, "*")}'


def get_previous_commit() -> str:
    """
    Retrieves the commit hash of the previous commit in the current Git
    repository.

    Returns:
    str: The hash of the previous commit. Returns an empty string if there is
    an error or if it's the initial commit.
    """
    result = subprocess.run(
        ['git', 'rev-parse', 'HEAD^'],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    return result.stdout.strip()


class Results(List[Tuple[str, int]]):
    """
    A class representing a list of test results.

    This class extends the List data structure to specifically handle test
    results. Each test result is a tuple containing the test name and its
    status (0 for success, non-zero for failure).
    """
    @property
    def total(self) -> int:
        return len(self)

    @property
    def success(self) -> int:
        return sum(1 for result in self if result[1] == 0)

    @property
    def errors(self) -> int:
        return sum(1 for result in self if result[1] != 0)

    @property
    def pass_rate(self) -> int:
        return int(self.success * 100 / self.total) if self.total != 0 else 0

    def __str__(self) -> str:
        out_str = 'Tests summary\n'

        for result in self:
            status = 'Success' if result[1] == 0 else 'Failed'
            out_str += f'{result[0]}: {status}\n'

        out_str += (
            f'Total: {self.success} / {self.total} '
            f'passed ({self.pass_rate}% pass rate)'
        )

        return out_str


def get_excluded_directories(exclude_dir_patterns):
    """
    Converts a list of directory names or patterns into a list of their
    absolute paths, intended to be used as an exclusion list.

    Parameters:
    exclude_dir_patterns (list): A list of directory names or patterns to
    exclude. These can be relative or absolute paths.

    Returns:
    list: A list of absolute paths corresponding to the input directory names
    or patterns.
    """

    filtered_dirs = list()
    for dir_pattern in exclude_dir_patterns:
        filtered_dirs.append(os.path.abspath(dir_pattern))
        print(f'\tAdding to the exclude list: {filtered_dirs[-1]}')

    return filtered_dirs


def get_filtered_files(exclude_dir_patterns, file_types):
    """
    Recursively find and filter files in the current working directory and its
    subdirectories.

    Parameters:
    exclude_dir_patterns (list): A list of directory paths patterns to exclude
    from the search.
    file_types (list): A list of file patterns to include in the result.

    Returns:
    list: A list of absolute paths to filtered files.
    """

    filtered_files = list()
    exclude_directories = get_excluded_directories(exclude_dir_patterns)
    cwd = os.getcwd()
    for root, dirs, files in os.walk(cwd, topdown=True):
        #
        # Exclude directories based on the EXCLUDE_DIRECTORIES pattern list
        #
        dirs[:] = list(filter(lambda d: os.path.join(root, d) not in
                              exclude_directories, dirs))

        #
        # List files based on the FILE_TYPES pattern list
        #
        files = list(filter(lambda f: any(fnmatch.fnmatch(f, pattern)
                            for pattern in file_types), files))

        #
        # Append all filtered files with the path
        #
        filtered_files.extend(list(map(lambda f: os.path.join(root, f),
                                       files)))

    return filtered_files
