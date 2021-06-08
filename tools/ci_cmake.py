#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import check_copyright
import check_doc
import check_spacing
import check_tabs
import docker
import os
import signal
import subprocess
import sys

from docker.errors import DockerException


def prod_variant(variant):
    return 'EXTRA_CONFIG_ARGS+=-DSCP_PLATFORM_VARIANT={}'.format(variant)


def banner(text):
    columns = 80
    title = " {} ".format(text)
    print("\n\n{}".format(title.center(columns, "*")))


def dockerize(client):
    (image, _) = client.images.build(path="docker", target="dev", rm=True)

    env = {"ARMLMD_LICENSE_FILE": os.environ["ARMLMD_LICENSE_FILE"]}

    volumes = {
        os.getcwd(): {
            "bind": "/scp-firmware",
            "mode": "rw",
        }
    }

    container = client.containers.run(
        image.id, __file__, environment=env, volumes=volumes, detach=True,
        tty=True
    )

    sigint_handler = signal.getsignal(signal.SIGINT)
    signal.signal(signal.SIGINT, lambda signal, _: container.kill(signal))

    logs = container.attach(stdout=True, stderr=True, stream=True, logs=True)
    for log in logs:
        sys.stdout.buffer.write(log)

    signal.signal(signal.SIGINT, sigint_handler)


def main():
    results = []

    try:
        client = docker.from_env()

        banner("Spawning container")

        try:
            return dockerize(client)
        except Exception as ex:
            print(ex)

            return 1
    except DockerException:
        pass

    banner("Code validation")

    result = check_copyright.main()
    results.append(("Check copyright", result))

    result = check_spacing.main()
    results.append(("Check spacing", result))

    result = check_tabs.main()
    results.append(("Check tabs", result))

    result = check_doc.main()
    results.append(("Check doc", result))

    banner("Build and run framework tests")

    result = subprocess.call("CC=gcc make clean test", shell=True)
    results.append(("Framework tests", result))

    banner("Test building host product")

    products = ['host', 'juno', 'morello', 'n1sdp', 'rdv1', 'rdv1mc',
                'rdn1e1', 'sgi575', 'sgm775', 'sgm776', 'tc0',
                'rdn2']
    platform_variant = {
        'rdn2': ['0']
    }

    build_types = ['debug', 'release']
    toolchains = ['GNU', 'ArmClang']

    for product in products:
        banner("Test building {} product".format(product))
        for toolchain in toolchains:
            if product == 'rcar' and toolchain == 'ArmClang':
                continue
            for build_type in build_types:

                cmd = 'make -f Makefile.cmake '
                cmd += 'PRODUCT={} TOOLCHAIN={} MODE={}'.format(product,
                                                                toolchain,
                                                                build_type)
                if product in platform_variant:
                    for variant in platform_variant[product]:
                        cmd_variant = cmd + ' ' + prod_variant(variant)
                        result = subprocess.call(cmd_variant, shell=True)
                        results.append(("Product {} build ({})".format(
                            product,
                            toolchain),
                            result))
                else:
                    result = subprocess.call(cmd, shell=True)
                    results.append(("Product {} build ({})".format(
                        product,
                        toolchain),
                        result))

    banner('Tests summary')

    total_success = 0
    for result in results:
        if result[1] == 0:
            total_success += 1
            verbose_result = "Success"
        else:
            verbose_result = "Failed"
        print("{}: {}".format(result[0], verbose_result))

    assert total_success <= len(results)

    print(
        "{} / {} passed ({}% pass rate)".format(
                                                total_success, len(results),
                                                int(total_success *
                                                    100 / len(results))
        )
    )

    if total_success < len(results):
        return 1
    else:
        return 0


if __name__ == "__main__":
    sys.exit(main())
