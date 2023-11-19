#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import argparse
import check_copyright
import check_doc
import check_spacing
import check_tabs
import check_framework
import check_module_utest
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
    check_module_utest,
    check_pycodestyle,
]

products = [
    Product('host', toolchains=[Parameter('GNU')]),
    Product('juno', variants=[Parameter('BOARD'), Parameter('FVP')]),
    Product('morello'),
    Product('n1sdp'),
    Product('rdv1', product_group='neoverse-rd'),
    Product('rdv1mc'),
    Product('rdn1e1', product_group='neoverse-rd'),
    Product('sgi575', product_group='neoverse-rd'),
    Product('sgm775'),
    Product('sgm776'),
    Product('synquacer'),
    Product('tc1', product_group='totalcompute'),
    Product('rcar', toolchains=[Parameter('GNU')]),
    Product('rdn2', variants=[
        Parameter('0'), Parameter('1'), Parameter('2'), Parameter('3')]),
    Product('rdfremont'),
    Product('tc2', variants=[
        Parameter('0'), Parameter('1')], product_group='totalcompute')
]


def banner(text):
    columns = 80
    title = " {} ".format(text)
    print("\n\n{}".format(title.center(columns, "*")))


def dockerize(client):
    (image, _) = client.images.build(path=".", dockerfile="docker/Dockerfile",
                                     target="dev", rm=True)

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


def do_build(build_info: List[Build], output_path: str) -> \
        List[Tuple[Build, subprocess.Popen]]:
    build_status: List[Tuple[Build, subprocess.Popen]] = []
    results: List[Tuple[str, int]] = []

    if not os.path.exists(output_path):
        os.makedirs(output_path)

    files = []
    for build in build_info:
        file_path = os.path.join(output_path, build.file_name())
        files.append(open(file_path, "w", encoding="utf-8"))

        build_id = subprocess.Popen(
                                build.command(),
                                shell=True,
                                stdout=files[-1],
                                stderr=subprocess.STDOUT)

        build_status.append((build, build_id))
        print('Test building [{}]'.format(build.tag()))
        print('[CMD] {}'.format(build.command()))

    for i, (build, build_id) in enumerate(build_status):
        build_id.communicate()
        results.append((build.tag(), build_id.returncode))
        files[i].close()
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


def check_errors(ignore_errors: bool, results: List[Tuple[str, int]]) -> bool:
    return not ignore_errors and len(list(filter(lambda x: x[1] != 0,
                                                 results)))


def main(ignore_errors: bool, skip_container: bool, log_level: str,
         output_path: str):
    # This code is only applicable if there is valid docker instance
    # On CI there is no docker instance at the moment
    if not skip_container:
        try:
            client = docker.from_env()
            banner("Spawning container")

            try:
                return dockerize(client)
            except Exception as ex:
                print(ex)
                return 1

        except DockerException:
            banner("Script running out of docker")
            pass
    else:
        banner("Skipping spawning container")

    results = []

    results.extend(code_validation(code_validations))
    if check_errors(ignore_errors, results):
        print('Errors detected! Excecution stopped')
        return analyze_results(*print_results(results))

    banner('Test building products')

    if output_path == "":
        output_path = os.path.join("/tmp", "scp")
    output_path = os.path.join(output_path, "build-output")

    for product in products:
        if log_level != "":
            product.log_level = Parameter(log_level)
        results.extend(do_build(product.builds, output_path))
        if check_errors(ignore_errors, results):
            print('Errors detected! Excecution stopped')
            return analyze_results(*print_results(results))

    return analyze_results(*print_results(results))


def parse_args():
    parser = argparse.ArgumentParser(
        description='Perform basic checks to SCP-Firmware and build for all \
                     supported platforms, modes and compilers.')

    parser.add_argument('-i', '--ignore-errors', dest='ignore_errors',
                        required=False, default=False, action='store_true',
                        help='Ignore errors and continue testing.')

    parser.add_argument('-ll', '--log-level', dest='log_level',
                        required=False, default="", type=str,
                        action='store', help='Build every product with the \
                        specified log level.')

    parser.add_argument('-bod', '--build-output-dir', dest='output_path',
                        required=False, default="", type=str, action='store',
                        help='Parent directory of the "build-output" directory\
                        , the one were the build logs will be stored in.\n \
                        If bod is not given, the default location is /tmp/scp/\
                        build-output')

    parser.add_argument('-sc', '--skip-container', dest='skip_container',
                        required=False, default=False, action='store_true',
                        help='Skip container execution.')

    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    sys.exit(main(args.ignore_errors, args.skip_container, args.log_level,
                  args.output_path))
