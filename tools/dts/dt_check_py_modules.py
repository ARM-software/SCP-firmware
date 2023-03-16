#!/usr/bin/env python3

# Copyright (c) 2023 Linaro Limited
# SPDX-License-Identifier: BSD-3-Clause

# script checks that the required python modules needed for dt processing are present.

import argparse
import importlib

required_modules = [ "devicetree", "pcpp", "cxxheaderparser" ]

def main():

    args = parse_input()
    mod_err = False

    if args.verbose:
        print("\n-----------------------------------\nChecking for required python modules\n")
    
    for mod in required_modules:
        spec = importlib.util.find_spec(mod)
        if spec is None:
            print(f"Error: unable to find module {mod}. Cannot run DT processing")
            mod_err = True
        else:
            if args.verbose:
                print(f"\nFound module {mod}, spec = {spec}")        
    if mod_err:
        raise Exception("Unable to find required module(s) for DT processing")

    if args.verbose:
        print("\npython module check complete\n-----------------------------------\n\n")

        
def parse_input():
    parser = argparse.ArgumentParser()

    parser.add_argument('--verbose', action='store_true', help="extra script logging for debug")

    return parser.parse_args()
    

# put in the main guard
if __name__ == "__main__":
    main()

    
