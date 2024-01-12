#!/usr/bin/env python3
#
# Arm SCP/MCP Software
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import yaml
from dataclasses import dataclass, field
from typing import List, Tuple


@dataclass(frozen=True, eq=True)
class Parameter:
    '''
    Parameter with its special arguments if needed
    '''
    name: str
    arguments: Tuple[str] = field(default_factory=tuple)

    @classmethod
    def from_yaml(cls, yaml_entry):
        param_list = list()
        for entry in yaml_entry:
            param_list.append(cls(entry['name'],
                              tuple(entry.get('extra_config_args', []))))

        return param_list


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
        filename += f'_{self.toolchain.name}_{self.build_type.name}'
        if self.log_level:
            filename += f'_{self.log_level.name}'
        if self.variant:
            filename += f'_{self.variant.name}'
        filename += '.txt'
        return filename

    def command(self, build_path=None):
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
        if build_path:
            cmd += f'BUILD_PATH={build_path} '
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

    @classmethod
    def from_yaml(cls, yaml_file):
        yaml_entries = yaml.safe_load(yaml_file)
        products = list()
        for yaml_entry in yaml_entries:
            product_properties = {'name': yaml_entry['product']}

            if 'product_group' in yaml_entry:
                pg = yaml_entry['product_group']
                product_properties['product_group'] = pg

            if 'variants' in yaml_entry:
                variants = yaml_entry['variants']
                product_properties['variants'] = \
                    Parameter.from_yaml(variants)

            if 'toolchains' in yaml_entry:
                toolchains = yaml_entry['toolchains']
                product_properties['toolchains'] = \
                    Parameter.from_yaml(toolchains)

            if 'build_types' in yaml_entry:
                build_types = yaml_entry['build_types']
                product_properties['build_types'] = \
                    Parameter.from_yaml(build_types)

            products.append(cls(**product_properties))

        return products

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
