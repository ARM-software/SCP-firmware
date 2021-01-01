#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Description:
#   This tool takes a list of module names and generates two files:
#   * fwk_modules_idx.h: Contains an enumeration giving the modules' indices.
#   * fwk_modules_list.c: Contains a table of pointers to a module descriptor.
#
# Note: The files are updated only if their contents will differ, relative to
#   the last time the tool was run.
#

import argparse
import os
import sys
import tempfile

DEFAULT_PATH = 'build/'

FILENAME_H = "fwk_module_idx.h"
TEMPLATE_H = "/* This file was auto generated using {} */\n" \
             "#ifndef FWK_MODULE_IDX_H\n" \
             "#define FWK_MODULE_IDX_H\n" \
             "\n" \
             "#include <fwk_id.h>\n" \
             "\n" \
             "enum fwk_module_idx {{\n" \
             "{}" \
             "}};\n" \
             "\n" \
             "{}" \
             "\n" \
             "#endif /* FWK_MODULE_IDX_H */\n"

FILENAME_C = "fwk_module_list.c"
TEMPLATE_C = "/* This file was auto generated using {} */\n" \
             "#include <stddef.h>\n" \
             "#include <fwk_module.h>\n" \
             "\n" \
             "{}" \
             "\n" \
             "const struct fwk_module *module_table[] = {{\n" \
             "{}" \
             "}};\n" \
             "\n" \
             "const struct fwk_module_config *module_config_table[] = {{\n" \
             "{}" \
             "}};\n"


def generate_file(path, filename, content):
    full_filename = os.path.join(path, filename)

    try:
        with open(full_filename) as f:
            rewrite = f.read() != content
    except FileNotFoundError:
        rewrite = True

    if rewrite:
        with tempfile.NamedTemporaryFile(prefix="gen-module-code",
                                         dir=path,
                                         delete=False,
                                         mode="wt") as f:
            print("[GEN] {}...".format(full_filename))
            f.write(content)
        os.replace(f.name, full_filename)


def generate_header(path, modules):
    enum = ""
    const = ""
    for idx, module in enumerate(modules):
        enum += "    FWK_MODULE_IDX_{} = {},\n".format(module.upper(), idx)
        const += "static const fwk_id_t fwk_module_id_{} = " \
                 "FWK_ID_MODULE_INIT(FWK_MODULE_IDX_{});\n".format(module,
                                                                   module
                                                                   .upper())

    enum += "    FWK_MODULE_IDX_COUNT = {},\n".format(idx + 1)

    content = TEMPLATE_H.format(sys.argv[0], enum, const)
    generate_file(path, FILENAME_H, content)


def generate_c(path, modules):
    module_entry = ""
    config_entry = ""
    extern_entry = ""
    for module in modules:
        extern_entry += "extern const struct fwk_module module_{};\n"\
            .format(module.lower())
        extern_entry += "extern const struct fwk_module_config config_{};\n"\
            .format(module.lower())
        module_entry += "    &module_{},\n".format(module.lower())
        config_entry += "    &config_{},\n".format(module.lower())

    content = TEMPLATE_C.format(sys.argv[0], extern_entry, module_entry,
                                config_entry)
    generate_file(path, FILENAME_C, content)


def main():
    parser = argparse.ArgumentParser(description="Generates a header file and \
        source file enumerating the modules that are included in a firmware.")

    parser.add_argument('modules',
                        metavar='module',
                        type=str,
                        nargs='*',
                        help='A list of module names that are included in a \
                            firmware.')

    parser.add_argument('-p', '--path',
                        help='Path to the location where generated files are \
                        written. If the files exist then they will be \
                        overwritten.',
                        default=DEFAULT_PATH)

    args = parser.parse_args()

    modules = args.modules

    generate_header(args.path, modules)
    generate_c(args.path, modules)


if __name__ == "__main__":
    main()
