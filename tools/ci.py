#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import check_copyright
import check_doc
import check_spacing
import check_tabs
import subprocess
import sys
import os


def banner(text):
    columns = 80
    title = ' {} '.format(text)
    print('\n\n{}'.format(title.center(columns, '*')))


def product_build(product, results, mode):

    banner('Test building {} product'.format(product))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT={product} ' \
        'MODE={mode} ' \
        'make all -j'.format(product=product, mode=mode)
    result = subprocess.call(cmd, shell=True)
    results.append(('Product {product} {mode} build (GCC)'.format(
        product=product, mode=mode), result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT={product} ' \
        'MODE={mode} ' \
        'make all -j'.format(product=product, mode=mode)
    result = subprocess.call(cmd, shell=True)
    results.append(('Product {product} {mode} build (ARM)'.format(
        product=product, mode=mode), result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT={product} ' \
        'MODE={mode} ' \
        'make all -j'.format(product=product, mode=mode)
    result = subprocess.call(cmd, shell=True)
    results.append(('Product {product} {mode} build (LLVM)'.format(
        product=product, mode=mode), result))

    subprocess.run('make clean', shell=True)


def product_build_debug_and_release(product, results):

    product_build(product, results, 'release')
    product_build(product, results, 'debug')


def main():
    results = []

    banner('Code validation')

    result = check_copyright.main()
    results.append(('Check copyright', result))

    result = check_spacing.main()
    results.append(('Check spacing', result))

    result = check_tabs.main()
    results.append(('Check tabs', result))

    result = check_doc.main()
    results.append(('Check doc', result))

    result = subprocess.call('python -m pycodestyle tools/', shell=True)
    results.append(('PyCodeStyle', result))

    banner('Build and run framework tests')

    result = subprocess.call('CC=gcc make clean test', shell=True)
    results.append(('Framework tests', result))

    banner('Test building host product')

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=gcc ' \
        'PRODUCT=host ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product host build (GCC)', result))

    product_build_debug_and_release('sgm775', results)
    product_build_debug_and_release('sgi575', results)
    product_build_debug_and_release('n1sdp', results)
    product_build_debug_and_release('rdn1e1', results)
    product_build_debug_and_release('juno', results)
    product_build_debug_and_release('synquacer', results)
    product_build_debug_and_release('sgm776', results)
    product_build_debug_and_release('rdv1', results)
    product_build_debug_and_release('rdv1mc', results)
    product_build_debug_and_release('rdn2', results)
    product_build_debug_and_release('tc0', results)
    product_build_debug_and_release('tc1', results)
    product_build_debug_and_release('morello', results)

    banner('Test building rcar product')

    # We need to set SYSROOT to </path/to/gcc-toolchain>/aarch64-none-elf
    # to make LLVM work properly
    SYS_ROOT_PATH = \
        os.path.abspath(
            subprocess.check_output(["which", "aarch64-none-elf-gcc"]).decode()
            + '/../../aarch64-none-elf'
        )

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=aarch64-none-elf-gcc ' \
        'PRODUCT=rcar ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rcar debug build (GCC-AArch64)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT='+SYS_ROOT_PATH+' ' \
        'CC=clang-11 ' \
        'PRODUCT=rcar ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rcar debug build (LLVM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=aarch64-none-elf-gcc ' \
        'PRODUCT=rcar ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rcar release build (GCC-AArch64)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT='+SYS_ROOT_PATH+' ' \
        'CC=clang-11 ' \
        'PRODUCT=rcar ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rcar release build (LLVM)', result))

    banner('Tests summary')

    total_success = 0
    for result in results:
        if result[1] == 0:
            total_success += 1
            verbose_result = 'Success'
        else:
            verbose_result = 'Failed'
        print('{}: {}'.format(result[0], verbose_result))

    assert total_success <= len(results)

    print('{} / {} passed ({}% pass rate)'.format(
        total_success,
        len(results),
        int(total_success * 100 / len(results))))

    if total_success < len(results):
        return 1
    else:
        return 0


if __name__ == '__main__':
    sys.exit(main())
