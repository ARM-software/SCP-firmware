#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

"""
    A wrapper around cppcheck which will be called from CMake's context.

    CMake doesn't give any mechanism to execute any programs or
    commands prior or after cppcheck's invocation.

    Usually CMAKE_C_CPPCHECK will contain the path to cppcheck binary
    on the build system, instead CMAKE_C_CPPCHECK will now contain
    path to this script which in turn calls cppcheck but this enables
    to inject any commands prior to and after actual invocation of
    cppcheck.

    One use case would be to mark cppcheck's invocation as below

    [CPPCHECK] ------>
    ....
    ....
    [CPPCHECK] <------

    This script also prints a warning if the version of the cppcheck
    does not match the required version.

"""

import sys
import subprocess
import re

# Holds required version info.
required_tool_version = "2.8"

tool_name = "CPPCHECK"


# Helper function
def print_msg(message, output):
    if output:
        print("[{}]  {}".format(tool_name, message))


def main():
    # CMake create a list of argument which include path of the
    # tool(e.g. cppcheck) binary and its arguments.
    tool_path = sys.argv[1]
    tool_args = ' '.join(sys.argv[2:])
    cmd = "{} {}".format(tool_path, tool_args)

    # Check if output of the tool (e.g. cppcheck) is to be displayed
    if re.search("verbose", tool_args):
        verbose = True
    else:
        verbose = False

    # If yes, then remove verbose string from argument list.
    if verbose:
        tool_args.replace("verbose", "")

    # Check version
    r = subprocess.Popen(tool_path + " --version",
                         shell=True,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)

    out, err = r.communicate()

    tool_version = out.decode("utf-8", "ignore")

    if out:
        print_msg("------>", output=verbose)
        print_msg("cppcheck {}"
                  " version {} ".format(tool_path, tool_version),
                  output=verbose)

        if not re.search(required_tool_version, tool_version):
            print_msg("Warning: recommended"
                      " {} version is {}"
                      " but found {}".format(tool_name,
                                             required_tool_version,
                                             tool_version),
                      output=True)
    if err:
        print_msg("Error: failed to find"
                  " binary at {}".format(tool_path),
                  output=True)
        return 1

    print_msg(cmd, output=verbose)

    r = subprocess.Popen(cmd,
                         shell=True,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)

    out, err = r.communicate()

    if out:
        print_msg("" + out.decode("utf-8", "ignore"), output=verbose)
    if err:
        print_msg("" + err.decode("utf-8", "ignore"), output=True)

    print_msg("<------", output=verbose)
    return r.returncode


if __name__ == '__main__':
    sys.exit(main())
