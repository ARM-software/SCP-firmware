#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import check_copyright
import check_doc
import check_spacing
import check_tabs
import check_framework
import check_pycodestyle
import docker
import os
import signal
import subprocess
import sys
from product import Product, Build, Parameter
from typing import List, Tuple

from docker.errors import DockerException


code_validations = [
    check_copyright,
    check_spacing,
    check_tabs,
    check_doc,
    check_framework,
    check_pycodestyle,
]

products = [
    Product('host', toolchains=[Parameter('GNU')]),
    Product('juno'),
    Product('morello'),
    Product('n1sdp'),
    Product('rdv1'),
    Product('rdv1mc'),
    Product('rdn1e1'),
    Product('sgi575'),
    Product('sgm775'),
    Product('sgm776'),
    Product('synquacer'),
    Product('tc0'),
    Product('tc1'),
    Product('rcar', toolchains=[Parameter('GNU')]),
    Product('rdn2', variants=[Parameter('0')]),
]


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


def code_validation(checks: list) -> List[Tuple[str, int]]:
    banner('Code validation')
    results: List[Tuple[str, int]] = []
    for check in checks:
        result = check.main()
        test_name = check.__name__.split('_')[-1]
        results.append(('Check {}'.format(test_name), result))
    return results


def start_build(build_info: List[Build]) -> List[Tuple[Build,
                                                       subprocess.Popen]]:
    build_status: List[Tuple[Build, subprocess.Popen]] = []
    for build in build_info:
        build_id = subprocess.Popen(
                                build.command(),
                                shell=True,
                                stdout=subprocess.DEVNULL,
                                stderr=subprocess.STDOUT)

        build_status.append((build, build_id))
        print('Test building [{}]'.format(build.tag()))
        print('[CMD] {}'.format(build.command()))
    return build_status


def wait_builds(build_status: Tuple[Build, subprocess.Popen]) -> List[Tuple[
                                                                        str,
                                                                        int]]:
    results: List[Tuple[str, int]] = []
    for build, build_id in build_status:
        result = build_id.wait()
        results.append((build.tag(), result))
    return results


def print_results(results: List[Tuple[str, int]]) -> Tuple[int, int]:
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
    return (total_success, len(results))


def analyze_results(success: int, total: int) -> int:
    print("{} / {} passed ({}% pass rate)".format(success, total,
                                                  int(success * 100 / total)))
    return 1 if success < total else 0


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

    results.extend(code_validation(code_validations))

    banner('Test building products')

    for product in products:
        build_status = start_build(product.builds)
        results.extend(wait_builds(build_status))

    return analyze_results(*print_results(results))


if __name__ == "__main__":
    sys.exit(main())
