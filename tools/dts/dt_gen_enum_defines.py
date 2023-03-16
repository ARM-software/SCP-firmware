#!/usr/bin/env python3

# Copyright (c) 2022 Linaro Limited
# SPDX-License-Identifier: BSD-3-Clause

# this script takes a number of  input C header file, preprocesses them through pcpp
# with supplied -D define and -I include inputs, then scans the resultant with
# cxxheaderparser to create a database of header classes.
# We then take the enums and #defines and output them
# all as defines to the output define header file.
#


import argparse
from collections import defaultdict
import os, io
import pathlib
import re
import sys
import pprint, dataclasses
import pcpp
from pcpp.preprocessor import Preprocessor, OutputDirective, Action
import cxxheaderparser
from cxxheaderparser.simple import parse_string
from cxxheaderparser.options import ParserOptions

#
# pcpp processor class modified for the device tree #define requirements.
#
class DTPreprocessor(Preprocessor):
    def __init__(self, argp):
        super(DTPreprocessor, self).__init__()

        # we always pass through defines
        self.passthru_defines = True
        self.printadefine = argp.print_n_defs
        
        # additional passthru option for __PCPP_ALWAYS.... macros
        self.bypass_ifpassthru = False

        if argp.debug_out:
            self.debugout = debug_out_file       

        # handle the defines inputs
        if argp.defines:
            in_defines = [x[0] for x in argp.defines]
            
            if verbose and argp.debug_out:
                print ("****************************************************************************", file = debug_out_file)
                print ("in_defines", in_defines, file = debug_out_file)

            for d in in_defines:
               if '=' not in d:
                    d += '=1'
               d = d.replace('=', ' ', 1)
               self.define(d)

            if verbose and argp.debug_out:
                print ("preprocessor in macros", self.macros, file = debug_out_file)
                print ("****************************************************************************", file = debug_out_file)

        # handle the include paths inputs
        if argp.includes:
            in_includes = [x[0] for x in argp.includes]

            if verbose and argp.debug_out:
                print ("****************************************************************************", file = debug_out_file)
                print ("in_includes", in_includes, file = debug_out_file)

            for i in in_includes:
                self.add_path(i)

            if verbose and argp.debug_out:
                print ("preprocessor include paths", self.path, file = debug_out_file)
                print ("****************************************************************************", file = debug_out_file)
                

    # we always pass through unfound includes
    def on_include_not_found(self,is_malformed,is_system_include,curdir,includepath):
        raise OutputDirective(Action.IgnoreAndPassThrough) # force the main preprocessor class to pass through.

    # handle directives including defines that get passed through
    def on_directive_handle(self,directive,toks,ifpassthru,precedingtoks):
        if ifpassthru:
            if directive.value == 'if' or directive.value == 'elif' or directive == 'else' or directive.value == 'endif':
                self.bypass_ifpassthru = len([tok for tok in toks if tok.value == '__PCPP_ALWAYS_FALSE__' or tok.value == '__PCPP_ALWAYS_TRUE__']) > 0
            if not self.bypass_ifpassthru and (directive.value == 'define' or directive.value == 'undef'):
                if toks[0].value != self.potential_include_guard:
                    raise OutputDirective(Action.IgnoreAndPassThrough)  # Don't execute anything with effects when inside an #if expr with undefined macro
        if self.passthru_defines:
            res = super(DTPreprocessor, self).on_directive_handle(directive,toks,ifpassthru,precedingtoks)
            if self.printadefine > 0 and directive.value == 'define' :
                print('super res = ', res, 'if pass through = ', ifpassthru)
                print('directive = ', directive)
                print('toks = ', toks)
                print('precedingtoks = ', precedingtoks)
                print('')
                self.printadefine =  self.printadefine - 1
            return None  # Pass through where possible
        return super(DTPreprocessor, self).on_directive_handle(directive,toks,ifpassthru,precedingtoks)

#
# pass a single header through the pre-processor
#
def preprocess_header(fname, pcpp_proc, out_str):
    infile = open(fname, "rt")
    pcpp_proc.parse(infile)
    pcpp_proc.write(out_str)
    infile.close()

    # if we have a debug out file then we actually write
    # a separate out file for the input.
    if debug_out_file:
        dbg_fname = fname_to_debug_path(fname, ".pcpp_out")
        outfile = open(dbg_fname, "wt")
        outfile.write(out_str.getvalue())
        outfile.close()

#
# cxx header parser - convert output string from pcpp to a cxx database and extract enums
#
def cxx_header_process(fname, pcpp_out_str):

    # additional debug info direct from library - can't be redirected to file so only
    # output if issues during development.
    if cxx_debug:
        optionsval = True
    else:
        optionsval = False

    cxx_str = cxx_remove_bad_keywords(fname, pcpp_out_str)
    cxx_str = cxx_remove_structs(fname, cxx_str)
    
    parsed_data = parse_string(cxx_str, options=ParserOptions(optionsval))
    if verbose:
        dbg_fname = fname_to_debug_path(fname, ".cxx_out")
        outfile = open(dbg_fname, "wt")
        ddata = dataclasses.asdict(parsed_data)
        pprint.pprint(ddata, stream = outfile, width=80, compact=True)
        outfile.close()

    # label straight defines block    
    s = f"""\
defines from file,
"""
    out_comment(s, True)

   
    for define in parsed_data.defines:
        # print defines to file - skip framework defines unless direct processing of a framework header
        # define name is first part of content
        dtoks = define.content.split()
        dname = dtoks[0]
        if fname.find('fwk') >= 0 or not dname.find('FWK_') >= 0:
            print('#define ', define.content, file=out_file)
        
    # now handle the enums...
    for enum in parsed_data.namespace.enums:
        process_cxx_enum(enum)

#
# handle the enums by generating defines for each element
#
def process_cxx_enum(enum):

    pqname = enum.typename
    if isinstance(pqname.segments[0], cxxheaderparser.types.NameSpecifier):
        enum_name = pqname.segments[0].name
        if enum_name.find('fwk') < 0:
            s = f"""generated from enum {enum_name}:\
"""
            out_comment(s, True)

            curr_val = 0
            prev_hex = False;
            prev_str_val = ''
            prev_str = False;
            
            for enum_val in enum.values:
                # standard incrementing value (or string + increment )
                if enum_val.value is None:

                    # set value to curr value and increment
                    if prev_str:
                        val_num = "( " + prev_str_val + " + " + str(curr_val) + " )"
                    else:    
                        val_num = curr_val
                        if prev_hex:
                            val_num = hex(val_num)
                    curr_val += 1

                # we need to evaluate the tokens
                else:
                    val_str = cxx_value_tokens_to_str(enum_val)
                    try:
                        eval_def  = cxx_eval_enum_val(enum_val)
                    except SyntaxError as err:
                        print(f'#error(\"Syntax Error ({err}) processing \'{val_str}\' as expression.\")', file = out_file)
                        return
                    
                    val_num = eval_def[0]

                    # value not a string name
                    if not eval_def[2]:
                        curr_val = val_num + 1
                        if eval_def[1] or prev_hex:
                            val_num = hex(val_num)
                            prev_hex = True
                    else:
                        prev_str = True
                        prev_str_val = val_num
                        curr_val = 1    # set increment for next str

                # output final value to file as a define
                print('#define ', enum_val.name, '    ', val_num, file = out_file)
                
#
# cxxheaderparser gets upset when the framework uses the C++ reserved word new as a
# parameter name, and uses the keyword restrict on pointers.
# Strip these out as we are not using this output for compilation.
# 
def cxx_remove_bad_keywords(fname, pcpp_out_str):
    
    stripped_str = pcpp_out_str.getvalue().replace("new", "_nw")
    stripped_str = stripped_str.replace(" restrict ", "")
    stripped_str = stripped_str.replace("*restrict ", "*")
    if verbose:
        dbg_fname = fname_to_debug_path(fname, ".cxx_nnew")
        nnew_file = open(dbg_fname, "wt")
        nnew_file.write(stripped_str)
        nnew_file.close()
    return stripped_str

# find next struct keyword in string - must be struct surrounded by whitespace - some var names
# contain struct within them!
def cxx_find_struct_kw(struct_str, st_pos):
    struct_pos = struct_str.find('struct', st_pos)
    if cxx_debug:
        print(f"found struct at {struct_pos}")
    while struct_pos >= 0:
        next_ch = struct_str[struct_pos + 6]
        if (struct_pos == 0):
            prev_ch = ' '
        else:
            prev_ch = struct_str[struct_pos - 1]
        
        if cxx_debug:
            print(f"prev_ch = {prev_ch}, next ch = {next_ch}")
            
        if prev_ch.isspace() and next_ch.isspace():
            if cxx_debug:
                print( f"valid struct kw at pos {struct_pos}")
            return struct_pos
        elif cxx_debug:           
            print(f"not valid struct kw at pos {struct_pos}")

        struct_pos = struct_str.find('struct', struct_pos + 6)
        if cxx_debug:
            print(f"found struct at {struct_pos}")

    if cxx_debug:
        print("no valid struct kw found")
    return struct_pos
        


# some structs in the SCP code are proving troublesome - remove all structs to be safe
def cxx_remove_structs(fname, pcpp_out_str):

    edt_str = pcpp_out_str
    struct_pos = cxx_find_struct_kw(edt_str, 0)

    while struct_pos >= 0:
        last_rbrack_pos = 0
        lbrack_pos = edt_str.find('{', struct_pos)
        rbrack_pos = edt_str.find('}', lbrack_pos)
        
        # check for single line struct
        semi_pos = edt_str.find(';', struct_pos) 
        if semi_pos > lbrack_pos:
            depth = 0
            while last_rbrack_pos <= 0 and depth < 10:
                if cxx_debug:
                    print(f"st lbrack {lbrack_pos}, rbrack {rbrack_pos}")
                    
                next_lbrack = edt_str.find('{', lbrack_pos + 1)
                if next_lbrack == -1 or next_lbrack > rbrack_pos:
                    last_rbrack_pos = rbrack_pos
                else:
                    depth += 1
                    lbrack_pos = next_lbrack
                    rbrack_pos = edt_str.find('}',rbrack_pos + 1)

                if cxx_debug:
                    print(f"en lbrack {lbrack_pos}, rbrack {rbrack_pos}, last_rbrack {last_rbrack_pos}, next_lbrack {next_lbrack}, depth {depth}")

            if depth >= 9:
                raise Exception("Too Deep searching for brackets")
        
        
            semi_pos = edt_str.find(';', last_rbrack_pos)
            rm_str = edt_str[0:struct_pos-1] + edt_str[semi_pos+1:]
            edt_str = rm_str
        else:
            struct_pos = semi_pos
            
        # restart where the last struct used to be!
        struct_pos = cxx_find_struct_kw(edt_str, struct_pos)

    if verbose:
        dbg_fname = fname_to_debug_path(fname, ".cxx_nstruct")
        nnew_file = open(dbg_fname, "wt")
        nnew_file.write(edt_str)
        nnew_file.close()
        
    return edt_str

def cxx_value_tokens_to_str(enum_val):
    val_str = ''
    for etoken in enum_val.value.tokens:
        val_str += etoken.value + ' '
    if cxx_debug:
        print(f"tokens = {enum_val.value.tokens}, str = {val_str}")
    return val_str


def cxx_eval_enum_val(enum_val):
    # if we see these in the expression then assume hex format
    bitwise_types = [ '|', '&', '<<', '>>' ]
    hex_fmt = False
    as_str = False
    val_str = ''
    is_expr = False
    
    for etoken in enum_val.value.tokens:
        val_str += etoken.value + ' '
        if etoken.type in bitwise_types:
            hex_fmt = True
        if etoken.type == 'NAME':
            as_str = True
        else:
            is_expr = True

    if '0x' in val_str:
        hex_fmt = True
            
    if as_str:
        # expression with a name token
        if is_expr:
            val_num = "( " + val_str + " )"
        else:
        # name token only    
            val_num = val_str
    else:
        # evaluatable value expression
        val_num = eval(val_str)

    return [val_num, hex_fmt, as_str]
        
    
    

#
# print output file helpers
#

# main header
def outfile_write_header(args):
    s = f"""\
File: {args.defines_file.name}

Generated Devicetree #defines file.

Contains enum values from source headers converted to #defines, 
plus other #defines extracted from the headers.

Allows same symbols to be used in device tree and source code.

Generated by: {__file__}

"""
    out_comment(s, False)

    s = f"""\

#ifndef {out_file_include_macro}
#define {out_file_include_macro}

#include <fwk_macros.h>

#include "fwk_dt_mod_idx_gen.h"

"""
    print(s, file=out_file)
    
# section header
def outfile_write_sec_hdr_filename(fname):
    s = f"""\
Output created from {fname}
"""
    out_comment(s)

# section    
def outfile_write_sec_footer(fname):
    s = f"""\
End of output from {fname}
"""
    out_comment(s)

# main footer    
def outfile_write_footer():
    s = f"""\

#endif /* {out_file_include_macro} */

"""
    print(s, file=out_file)
    
#
# output file write input string commentised.
#
def out_comment(s, blank_before=True):
    # Writes 's' as a comment to the header and configuration file. 's' is
    # allowed to have multiple lines. blank_before=True adds a blank line
    # before the comment.
    
    if blank_before:
        print(file=out_file)

    if "\n" in s:
        # Format multi-line comments like
        #
        #   /*
        #    * first line
        #    * second line
        #    *
        #    * empty line before this line
        #    */
        res = ["/*"]
        for line in s.splitlines():
            # Avoid an extra space after '*' for empty lines. They turn red in
            # Vim if space error checking is on, which is annoying.
            res.append(" *" if not line.strip() else " * " + line)
        res.append(" */")
        print("\n".join(res), file=out_file)
    else:
        # Format single-line comments like
        #
        #   /* foo bar */
        print("/* " + s + " */", file=out_file)
       
# take the file name part of fname, plug in debug dir, add suffix
def fname_to_debug_path(fname, suffix):

    if (debug_dir is not None):
        file_name = pathlib.Path(fname).name
        file_name += suffix
        new_path = pathlib.Path(debug_dir).joinpath(file_name)
    else:
        new_path = fname + suffix
#    print(f' ====> new path = {new_path}')
    return new_path
    
        
#
# main processing routine                
#
def main():

    global out_file
    global out_file_include_macro
    global verbose
    global debug_out_file
    global debug_dir
    global cxx_debug
    
    # process the command line input
    args = parse_input()

    verbose = args.verbose
    debug_out_file = args.debug_out
    cxx_debug = args.cxx_debug
    
    if verbose:
        print("Generating #defines file with following parameters")
        print("using:header in;        ", args.in_headers)
        print("     :defines-file out; ", args.defines_file)
        print("     :debug-out;        ", args.debug_out)
        print("     :defines;          ", args.defines)
        print("     :includes;         ", args.includes)
        print("     :print-n-defs;     ", args.print_n_defs)
        print("     :debug-dir;        ", args.debug_dir)

    # global debug directory for script output
    if args.debug_dir:
        debug_dir = pathlib.Path(args.debug_dir[0])
        debug_dir.mkdir(parents = True, exist_ok = True)
    else:
        debug_dir = None

    # open the debug out file
    if args.debug_out:
        dbg_out_fname = fname_to_debug_path(args.debug_out, "")
        debug_out_file = open(dbg_out_fname, "wt")
    
    # create the preprocessor, pass on some of the command line input if needed.
    pcpp_proc = DTPreprocessor(args)
    
    # write the header to the output file
    out_file = args.defines_file
    out_file_include_macro =  "_FWK_DT_GEN_DEFINES_H_"
    outfile_write_header(args)
    
    # process each input header
    for fname in args.in_headers:
        pcpp_out_str = io.StringIO()
        preprocess_header(fname, pcpp_proc, pcpp_out_str)
        if verbose and args.debug_out:
            print("****************************************************************************", file = debug_out_file)
            print("******", fname, " Processed output => *" , file = debug_out_file)       
            print(pcpp_out_str.getvalue(), file = debug_out_file)
            print("****************************************************************************", file = debug_out_file)

            
        # write section header comment for current input file
        outfile_write_sec_hdr_filename(fname)
        
        # process file string - pass to cxxheaderprocessor
        cxx_header_process(fname, pcpp_out_str)

        # set section footer for this input file
        outfile_write_sec_footer(fname)
        pcpp_out_str.close()

    # close the output file
    outfile_write_footer()
    out_file.close()

    # close any debug file
    if args.debug_out:
        debug_out_file.close()
    

#
# input options
#
def parse_input():
    # parse the command line options
    parser = argparse.ArgumentParser()

    parser.add_argument('--defines-file', type = argparse.FileType('wt'),  default='fwk_dt_gen_defines.h', help="the output defines header")
    parser.add_argument('--debug-out', const='dt_enum_gen_debug.out', nargs = '?', help="debug file for output from the dt_gen_enum_defines script")
    parser.add_argument('--debug-dir', nargs=1, help="debug dir for all script debugging output")
    parser.add_argument('in_headers', nargs='+', help="names of all the header files to process")
    parser.add_argument('--print-n-defs', nargs = 1, type = int, default = 0,  help='number of defines to print in the pcpp phase for debug')
    parser.add_argument('--verbose', action='store_true', help="extra script logging for debug")
    parser.add_argument('-D', dest = 'defines', metavar = 'macro[=val]', nargs = 1, action = 'append', help = 'Predefine name as a macro [with value]')
    parser.add_argument('-I', dest = 'includes', metavar = 'path', nargs = 1, action = 'append', help = "Path to search for unfound #include's")
    parser.add_argument('--cxx-debug', action='store_true', help="extra debugging output for cxx header parser (not redirected to file)")

    
    return parser.parse_args()

# put in the main guard
if __name__ == "__main__":
    main()
