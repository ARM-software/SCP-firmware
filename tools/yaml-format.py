#!/usr/bin/env python3

#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import argparse
import colorama
import difflib
import os
import sys

import ruamel.yaml as yaml

from io import StringIO


def colorize(diff):
    for line in diff:
        if line.startswith("@"):
            yield colorama.Fore.BLUE + line + colorama.Fore.RESET
        elif line.startswith("+"):
            yield colorama.Fore.GREEN + line + colorama.Fore.RESET
        elif line.startswith("-"):
            yield colorama.Fore.RED + line + colorama.Fore.RESET
        else:
            yield line


def main():
    # fmt: off

    parser = argparse.ArgumentParser()

    parser_common = argparse.ArgumentParser(add_help=False)
    parser_common.add_argument("sources",
                               help="list of source files to format or check",
                               nargs="*")
    parser_common.add_argument("-q", "--quiet", action="store_true",
                               help="suppress output intended for humans")

    subparsers = parser.add_subparsers(dest="command")

    subparser_diff = subparsers.add_parser("diff",
                                           help=("generate a unified diff " +
                                                 "of required changes"),
                                           parents=[parser_common])

    subparser_diff.add_argument("--check",
                                action="store_true",
                                help=("exit with an error status code " +
                                      "if changes are required"))

    subparser_diff.add_argument("-o", "--output",
                                help="file to write the unified diff file to")

    subparser_format = subparsers.add_parser("format",
                                             help=("automatically format " +
                                                   "source files"),
                                             parents=[parser_common])

    subparser_format_output_group = \
        subparser_format.add_mutually_exclusive_group()
    subparser_format_output_group.add_argument("-i", "--in-place",
                                               action="store_true",
                                               help="format the file in-place")
    subparser_format_output_group.add_argument("-o", "--output",
                                               help=("file to write " +
                                                     "output data to"))

    # fmt: on

    args = parser.parse_args()

    colorama.init

    result = os.EX_OK

    for source in args.sources:
        source = os.path.relpath(source)

        with open(source, "r") as istream:
            idata = istream.read()

        data = yaml.round_trip_load(idata)

        with StringIO() as odata:
            yaml.round_trip_dump(data, odata)

            odata = odata.getvalue()

        if ("in_place" in args) and args.in_place:
            ostream = open(source, "w")
        elif ("output" in args) and args.output:
            ostream = open(args.output, "w")
        else:
            ostream = sys.stdout

        if args.command == "diff":
            if idata != odata:
                if not args.quiet:
                    print("Style violations found: " + source, file=sys.stderr)

                if args.check:
                    result = os.EX_DATAERR

                diff = difflib.unified_diff(
                    idata.splitlines(keepends=True),
                    odata.splitlines(keepends=True),
                    fromfile=source + " (original)",
                    tofile=source + " (reformatted)",
                )

                for line in colorize(diff):
                    ostream.write(line)
        elif args.command == "format":
            ostream.write(odata)

        if ostream != sys.stdout:
            ostream.close()

    return result


if __name__ == "__main__":
    sys.exit(main())
