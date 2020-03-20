#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import check_copyright
import check_doc
import check_spacing
import check_tabs
import subprocess
import sys


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

    banner('Test building the framework library')

    cmd = \
        'CC=gcc ' \
        'BS_FIRMWARE_CPU=host ' \
        'make clean lib-framework -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Framework build (Host, GCC)', result))

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'BS_FIRMWARE_CPU=cortex-m3 ' \
        'make clean lib-framework -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Framework build (Cortex-M3, GCC)', result))

    cmd = \
        'CC=armclang ' \
        'BS_FIRMWARE_CPU=cortex-m3 ' \
        'make clean lib-framework -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Framework build (Cortex-M3, ARM)', result))

    banner('Test building arch library')

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'BS_FIRMWARE_CPU=cortex-m3 ' \
        'make clean lib-arch -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Arch build (Cortex-M3, GCC)', result))

    cmd = \
        'CC=armclang ' \
        'BS_FIRMWARE_CPU=cortex-m3 ' \
        'make clean lib-arch -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Arch build (Cortex-M3, ARM)', result))

    banner('Test building host product')

    cmd = \
        'CC=gcc ' \
        'PRODUCT=host ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product host build (GCC)', result))

    banner('Test building sgm775 product')

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=sgm775 ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm775 release build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=sgm775 ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm775 release build (ARM)', result))

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=sgm775 ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm775 debug build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=sgm775 ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm775 debug build (ARM)', result))

    banner('Test building sgi575 product')

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=sgi575 ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgi575 release build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=sgi575 ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgi575 release build (ARM)', result))

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=sgi575 ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgi575 debug build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=sgi575 ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgi575 debug build (ARM)', result))

    banner('Test building n1sdp product')

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=n1sdp ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product n1sdp debug build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=n1sdp ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product n1sdp debug build (ARM)', result))

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=n1sdp ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product n1sdp release build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=n1sdp ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product n1sdp release build (ARM)', result))

    banner('Test building rdn1e1 product')

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=rdn1e1 ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdn1e1 release build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=rdn1e1 ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdn1e1 release build (ARM)', result))

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=rdn1e1 ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdn1e1 debug build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=rdn1e1 ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rdn1e1 debug build (ARM)', result))

    banner('Test building juno product')

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=juno ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product Juno debug build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=juno ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product Juno debug build (ARM)', result))

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=juno ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product Juno release build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=juno ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product Juno release build (ARM)', result))

    banner('Test building synquacer product')

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=synquacer ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product synquacer release build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=synquacer ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product synquacer release build (ARM)', result))

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=synquacer ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product synquacer debug build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=synquacer ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product synquacer debug build (ARM)', result))

    banner('Test building sgm776 product')

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=sgm776 ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm776 release build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=sgm776 ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm776 release build (ARM)', result))

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=sgm776 ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm776 debug build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=sgm776 ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm776 debug build (ARM)', result))

    banner('Test building rddaniel product')

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=rddaniel ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rddaniel release build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=rddaniel ' \
        'MODE=release ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rddaniel release build (ARM)', result))

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=rddaniel ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rddaniel debug build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=rddaniel ' \
        'MODE=debug ' \
        'make clean all -j'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product rddaniel debug build (ARM)', result))

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
