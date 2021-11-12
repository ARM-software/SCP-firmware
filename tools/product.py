#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

from dataclasses import dataclass, field
from typing import List, Tuple


@dataclass(frozen=True, eq=True)
class Parameter:
    '''
    Parameter with its special arguments if needed
    '''
    name: str
    arguments: Tuple[str] = field(default_factory=tuple)


@dataclass(frozen=True, eq=True)
class Build:
    '''
    Specific build configuration
    '''
    name: str
    toolchain: Parameter
    build_type: Parameter
    variant: Parameter = None

    def tag(self):
        build_str = 'Product: {} - Toolchain: {} - Mode: {}'.format(
                                                        self.name,
                                                        self.toolchain.name,
                                                        self.build_type.name)
        if (self.variant):
            build_str += ' - Variant {}'.format(self.variant.name)
        return build_str

    def command(self):
        cmd = 'make -f Makefile.cmake '
        cmd += 'PRODUCT={} TOOLCHAIN={} MODE={} '.format(self.name,
                                                         self.toolchain.name,
                                                         self.build_type.name)
        if self.variant:
            cmd += Build.__extra_arg__('\"-DSCP_PLATFORM_VARIANT={}\"'.format(
                                                            self.variant.name))
            for extra in self.variant.arguments:
                cmd += Build.__extra_arg__(extra)
        for extra in self.toolchain.arguments:
            cmd += Build.__extra_arg__(extra)
        for extra in self.build_type.arguments:
            cmd += Build.__extra_arg__(extra)
        cmd += ' -j$(nproc) '
        return cmd

    @classmethod
    def __extra_arg__(cls, parameter: str) -> str:
        return 'EXTRA_CONFIG_ARGS+={} '.format(parameter)


@dataclass
class Product:
    '''
    This class defines platform build parameters to simplify configuration
    for testing. It also generates build commands.
    '''
    name: str
    toolchains: List[Parameter] = field(default_factory=lambda: [
        Parameter('GNU'),
        Parameter('ArmClang'),
        ])
    build_types: List[Parameter] = field(default_factory=lambda: [
        Parameter('debug'),
        Parameter('release'),
        ])
    variants: List[Parameter] = field(default_factory=list)

    @property
    def builds(self) -> List[Build]:
        builds = []
        for toolchain in self.toolchains:
            for build_type in self.build_types:
                if self.variants:
                    for variant in self.variants:
                        builds.append(Build(
                            self.name,
                            toolchain,
                            build_type,
                            variant))
                else:
                    builds.append(Build(
                        self.name,
                        toolchain,
                        build_type))
        return builds
