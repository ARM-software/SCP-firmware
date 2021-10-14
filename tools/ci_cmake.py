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
import check_framework
import docker
import os
import signal
import subprocess
import sys

from docker.errors import DockerException


code_validations = [
    check_copyright,
    check_spacing,
    check_tabs,
    check_doc,
    check_framework,
]

build_types = [
    'debug',
    'release',
]

toolchains = [
    'GNU',
    'ArmClang',
]

products = [
    'host',
    'juno',
    'morello',
    'n1sdp',
    'rdv1',
    'rdv1mc',
    'rdn1e1',
    'sgi575',
    'sgm775',
    'sgm776',
    'synquacer',
    'tc0',
    'rcar',
    'rdn2',
]

platform_variant = {
    'rdn2': [
        '0'
    ],
}


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


def generate_build_info():
    build_info = []
    for product in products:
        for toolchain in toolchains:
            if product == 'rcar' and toolchain == 'ArmClang':
                continue
            for build_type in build_types:

                cmd = 'make -f Makefile.cmake '
                cmd += 'PRODUCT={} TOOLCHAIN={} MODE={}'.format(product,
                                                                toolchain,
                                                                build_type)
                cmd += ' -j$(nproc) '

                if product in platform_variant:
                    for variant in platform_variant[product]:
                        cmd += ' ' + prod_variant(variant)
                        desc = "Product {}.{} build ({})".format(
                            product,
                            variant,
                            toolchain)
                        build_info.append((product, desc, cmd))
                else:
                    desc = "Product {} build ({})".format(
                            product,
                            toolchain)
                    build_info.append((product, desc, cmd))
    return build_info


def start_build(build_info):
    build_status = []
    for product, desc, cmd in build_info:
        build = subprocess.Popen(
                                cmd,
                                shell=True,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE)
        build_status.append((product, desc, build))
    return build_status


def main():
    # This code is only applicable if there is valid docker instance
    # On CI there is no docker instance at the moment
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

    results = []

    banner('Code validation')

    for code_validation in code_validations:
        result = code_validation.main()
        test_name = code_validation.__name__.split('_')[-1]
        results.append(('Check {}'.format(test_name), result))

    build_status = start_build(generate_build_info())

    for product, desc, build in build_status:
        banner('Test building {} product'.format(product))
        (stdout, stderr) = build.communicate()
        print(stdout.decode())
        print(stderr.decode())
        result = build.wait()
        results.append((desc, result))

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
