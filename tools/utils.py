#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import subprocess
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
