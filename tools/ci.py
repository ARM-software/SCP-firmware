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

    banner('Test building sgm775 product')

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=sgm775 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm775 release build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=sgm775 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm775 release build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=sgm775 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm775 release build (LLVM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=sgm775 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm775 debug build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=sgm775 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm775 debug build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=sgm775 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm775 debug build (LLVM)', result))

    banner('Test building sgi575 product')

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=sgi575 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgi575 release build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=sgi575 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgi575 release build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=sgi575 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgi575 release build (LLVM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=sgi575 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgi575 debug build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=sgi575 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgi575 debug build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=sgi575 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgi575 debug build (LLVM)', result))

    banner('Test building n1sdp product')

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=n1sdp ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product n1sdp debug build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=n1sdp ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product n1sdp debug build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=n1sdp ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product n1sdp debug build (LLVM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=n1sdp ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product n1sdp release build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=n1sdp ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product n1sdp release build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=n1sdp ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product n1sdp release build (LLVM)', result))

    banner('Test building rdn1e1 product')

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=rdn1e1 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdn1e1 release build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=rdn1e1 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdn1e1 release build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=rdn1e1 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdn1e1 release build (LLVM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=rdn1e1 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdn1e1 debug build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=rdn1e1 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdn1e1 debug build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=rdn1e1 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdn1e1 debug build (LLVM)', result))

    banner('Test building juno product')

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=juno ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product Juno debug build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=juno ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product Juno debug build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=juno ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product juno debug build (LLVM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=juno ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product Juno release build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=juno ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product Juno release build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=juno ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product juno release build (LLVM)', result))

    banner('Test building synquacer product')

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=synquacer ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product synquacer release build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=synquacer ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product synquacer release build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=synquacer ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product synquacer release build (LLVM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=synquacer ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product synquacer debug build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=synquacer ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product synquacer debug build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=synquacer ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product synquacer debug build (LLVM)', result))

    banner('Test building sgm776 product')

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=sgm776 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm776 release build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=sgm776 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm776 release build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=sgm776 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm776 release build (LLVM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=sgm776 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm776 debug build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=sgm776 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm776 debug build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=sgm776 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm776 debug build (LLVM)', result))

    banner('Test building rdv1 product')

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=rdv1 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdv1 release build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=rdv1 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdv1 release build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=rdv1 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdv1 release build (LLVM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=rdv1 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdv1 debug build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=rdv1 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdv1 debug build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=rdv1 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdv1 debug build (LLVM)', result))

    banner('Test building rdv1mc product')

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=rdv1mc ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdv1mc release build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=rdv1mc ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdv1mc release build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=rdv1mc ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdv1mc release build (LLVM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=rdv1mc ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdv1mc debug build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=rdv1mc ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdv1mc debug build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=rdv1mc ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdv1mc debug build (LLVM)', result))

    banner('Test building rdn2 product')

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=rdn2 ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdn2 release build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=rdn2 ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdn2 release build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=rdn2 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdn2 release build (LLVM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=rdn2 ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdn2 debug build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=rdn2 ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdn2 debug build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=rdn2 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdn2 debug build (LLVM)', result))

    banner('Test building tc0 product')

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=tc0 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product tc0 release build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=tc0 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product tc0 release build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=tc0 ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product tc0 release build (LLVM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=tc0 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product tc0 debug build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=tc0 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product tc0 debug build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=tc0 ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product tc0 debug build (LLVM)', result))

    banner('Test building morello product')

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=morello ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product morello debug build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=morello ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product morello debug build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=morello ' \
        'MODE=debug ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product morello debug build (LLVM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=morello ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product morello release build (GCC)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'CC=armclang ' \
        'PRODUCT=morello ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product morello release build (ARM)', result))

    subprocess.run('make clean', shell=True)

    cmd = \
        'SYSROOT_CC=arm-none-eabi-gcc ' \
        'CC=clang-11 ' \
        'PRODUCT=morello ' \
        'MODE=release ' \
        'make all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product morello release build (LLVM)', result))

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
