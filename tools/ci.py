#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
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
        'make clean lib-framework'
    result = subprocess.call(cmd, shell=True)
    results.append(('Framework build (Host, GCC)', result))

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'BS_FIRMWARE_CPU=cortex-m3 ' \
        'make clean lib-framework'
    result = subprocess.call(cmd, shell=True)
    results.append(('Framework build (Cortex-M3, GCC)', result))

    cmd = \
        'CC=armclang ' \
        'BS_FIRMWARE_CPU=cortex-m3 ' \
        'make clean lib-framework'
    result = subprocess.call(cmd, shell=True)
    results.append(('Framework build (Cortex-M3, ARM)', result))

    banner('Test building arch library')

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'BS_FIRMWARE_CPU=cortex-m3 ' \
        'make clean lib-arch'
    result = subprocess.call(cmd, shell=True)
    results.append(('Arch build (Cortex-M3, GCC)', result))

    cmd = \
        'CC=armclang ' \
        'BS_FIRMWARE_CPU=cortex-m3 ' \
        'make clean lib-arch'
    result = subprocess.call(cmd, shell=True)
    results.append(('Arch build (Cortex-M3, ARM)', result))

    banner('Test building host product')

    cmd = \
        'CC=gcc ' \
        'PRODUCT=host ' \
        'make clean all'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product host build (GCC)', result))

    banner('Test building sgm775 product')

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=sgm775 ' \
        'MODE=release ' \
        'make clean all'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm775 release build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=sgm775 ' \
        'MODE=release ' \
        'make clean all'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm775 release build (ARM)', result))

    cmd = \
        'CC=arm-none-eabi-gcc ' \
        'PRODUCT=sgm775 ' \
        'MODE=debug ' \
        'make clean all'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm775 debug build (GCC)', result))

    cmd = \
        'CC=armclang ' \
        'PRODUCT=sgm775 ' \
        'MODE=debug ' \
        'make clean all'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgm775 debug build (ARM)', result))

    banner('Test building sgi575 product')

    cmd = \
        'CC=arm-none-eabi-gcc CROSS_COMPILE=arm-none-eabi- ' \
        'PRODUCT=sgi575 ' \
        'make clean all'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product sgi575 build (GCC)', result))

    banner('Test building n1sdp product')

    cmd = \
        'CC=arm-none-eabi-gcc CROSS_COMPILE=arm-none-eabi- ' \
        'PRODUCT=n1sdp ' \
        'make clean all'
    result = subprocess.call(cmd, shell=True)
    results.append(('Product n1sdp build (GCC)', result))

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
