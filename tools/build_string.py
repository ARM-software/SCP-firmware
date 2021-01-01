#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import subprocess
from datetime import datetime


def main():
    # Check if git is available and that the source is within a git repository
    try:
        cmd = 'git describe --tags --dirty --always'
        output = subprocess.check_output(cmd.split())
        git_string = output.decode('utf-8').strip()
    except BaseException:
        git_string = 'Unknown'

    print("{}-{}".format(datetime.now().strftime('%Y-%m-%d_%H-%M-%S'),
          git_string))


if __name__ == '__main__':
    main()
