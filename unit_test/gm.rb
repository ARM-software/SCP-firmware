#!/usr/bin/ruby
#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

GIT_ROOT = `git rev-parse --show-toplevel | tr -d '\n'`
require "#{GIT_ROOT}/contrib/cmock/git/lib/cmock.rb"
require 'optparse'
require 'optparse/time'

files = []
cmock_options = {}
cmock_options[:verbosity] = 3

cmock_option_cfg =  "#{GIT_ROOT}/unit_test/cfg.yml"

OptionParser.new do |parser|
    parser.on('-h', '--help', 'Print this help') do |help|
      puts "Usage gm.rb [options]"
      puts parser
      exit
    end

    parser.on('-m', '--mock_path=PATH', 'Path where mock files are generated') do |mock_path|
        cmock_options[:mock_path] = mock_path
    end
    parser.on('-f', '--file_name=PATH', "Header file to generate mocks for") do |file_name|
        files << file_name
    end
    parser.on('-o', '--cmock_option_cfg=yaml_file', 'Mock configuration file') do |new_cfg|
        cmock_option_cfg = new_cfg
    end
    parser.on('-s', '--mock_suffix=mock_file_suffix', 'Mock file suffix') do |mock_suffix|
      cmock_options[:mock_suffix] = mock_suffix
    end
    parser.on('-d', '--subdir=mock_subdir', 'subdir') do |subdir|
      cmock_options[:subdir] = subdir
    end
end.parse!

cmock_options.merge!  CMockConfig.load_config_file_from_yaml(cmock_option_cfg)

cmock = CMock.new(cmock_options).setup_mocks(files)

# Create .clang_format to exclude mock_path from code styling
File.open(cmock_options[:mock_path] + "/.clang-format", 'w') do |f|
    f << "{\n \"DisableFormat\": true,\n \"SortIncludes\": false,\n}\n"
end
