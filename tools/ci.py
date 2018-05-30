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

    result = subprocess.call('pep8 tools/', shell=True)
    results.append(('PEP8', result))

    banner('Build and run framework tests')

    result = subprocess.call('make clean test', shell=True)
    results.append(('Framework tests', result))

    banner('Test building the framework library')

    cmd = 'CROSS_COMPILE= BS_FIRMWARE_CPU=host make lib-framework'
    result = subprocess.call(cmd, shell=True)
    results.append(('Framework build (host)', result))

    cmd = 'CROSS_COMPILE=arm-none-eabi- BS_FIRMWARE_CPU=cortex-m3 make '\
          'lib-framework'
    result = subprocess.call(cmd, shell=True)
    results.append(('Framework build (Cortex-M3)', result))

    banner('Test building arch library')

    cmd = 'CROSS_COMPILE=arm-none-eabi- BS_FIRMWARE_CPU=cortex-m3 make '\
          'lib-arch'
    result = subprocess.call(cmd, shell=True)
    results.append(('Arch build (Cortex-M3)', result))

    banner('Test building host product')

    cmd = 'CROSS_COMPILE= PRODUCT=host make '
    result = subprocess.call(cmd, shell=True)
    results.append(('Product host build', result))

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
