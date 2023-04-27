#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
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
    log_level: Parameter
    product_group: str
    variant: Parameter = None

    def tag(self):
        name = self.product_group + '/' + self.name if self.product_group \
            else self.name
        build_str = 'Product: {} - Toolchain: {} - Mode: {}'.format(
                                                        name,
                                                        self.toolchain.name,
                                                        self.build_type.name)
        if self.variant:
            build_str += ' - Variant {}'.format(self.variant.name)
        if self.log_level:
            build_str += ' - Log Level {}'.format(self.log_level.name)
        return build_str

    def file_name(self):
        filename = self.product_group + '_' + self.name if self.product_group \
            else self.name
        filename += "_" + self.toolchain.name + "_" + \
            self.build_type.name[0]
        if self.log_level:
            filename += "_" + self.log_level.name
        if self.variant:
            filename += "_" + self.variant.name[0]
        filename += ".txt"
        return filename

    def command(self):
        cmd = 'make -f Makefile.cmake '
        name = self.product_group + '/' + self.name if self.product_group \
            else self.name
        cmd += 'PRODUCT={} TOOLCHAIN={} MODE={} '.format(
                                                   name,
                                                   self.toolchain.name,
                                                   self.build_type.name)
        if self.log_level:
            cmd += 'LOG_LEVEL={} '.format(self.log_level.name)
        if self.variant:
            cmd += 'PLATFORM_VARIANT=\"{}\" '.format(self.variant.name)
            for extra in self.variant.arguments:
                cmd += Build.__extra_arg__(extra)
        for extra in self.toolchain.arguments:
            cmd += Build.__extra_arg__(extra)
        for extra in self.build_type.arguments:
            cmd += Build.__extra_arg__(extra)
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
        Parameter('Clang', ('-DSCP_LLVM_SYSROOT_CC=arm-none-eabi-gcc',)),
        ])
    build_types: List[Parameter] = field(default_factory=lambda: [
        Parameter('debug'),
        Parameter('release'),
        ])
    variants: List[Parameter] = field(default_factory=lambda: [None])
    log_level: Parameter = None
    product_group: str = field(default=None)

    @property
    def builds(self) -> List[Build]:
        builds = []
        for toolchain in self.toolchains:
            for build_type in self.build_types:
                for variant in self.variants:
                    builds.append(Build(
                        self.name,
                        toolchain,
                        build_type,
                        self.log_level,
                        self.product_group,
                        variant))
        return builds
