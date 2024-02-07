#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import re

"""
Filters out test and mock code from coverage report so that
only actual SCP code is evaluated
"""

Filename = "scp_v2_fwk_test_coverage.info"
OutFileName = "scp_v2_fwk_test_coverage_filtered.info"

STARTER = "SF:"
regexes = [re.compile(r"framework/test/")]

ENDED = "end_of_record"


def get_filtered_info():
    within = False
    filter = False
    dump_lines = []

    with open(OutFileName, "w") as fd:
        with open(Filename, "r") as fd_in:
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


if __name__ == "__main__":
    get_filtered_info()
