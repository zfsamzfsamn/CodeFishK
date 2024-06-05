#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Copyright (c) 2021, Huawei Device Co., Ltd. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of Willow Garage, Inc. nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import os
import re
from string import Template

from .hdf_dot_config_file import HdfDotConfigFile
import hdf_utils


class HdfModuleKconfigFile(object):
    def __init__(self, root, module, k_path):
        self.root = root
        self.module = module
        self.k_path = k_path
        self.module_models = {
            'self': {},
            'children': []
        }
        self.lines = []
        self.dot_config = None
        self.config_re = re.compile(r'\s*config\s+([a-zA-Z0-9_\-]+)')
        self.depends_on_re = re.compile(r'depends\s+on\s+([a-zA-Z0-9_\-]+)')
        self.choice_re = re.compile(r'^\s*choice\s*$')
        self.endchoice_re = re.compile(r'^\s*endchoice\s*$')
        self.config_splitter = 'DRIVERS_HDF_'
        self.top_res = [
            (self.config_re, self._parse_config),
            (self.choice_re, self._parse_choice),
            (self.endchoice_re, None)
        ]

    def _add_model(self, name_, config_item_, depends_on_item_, enabled_):
        model = {'name': name_,
                 'config_item': config_item_,
                 'depends_on_item': depends_on_item_,
                 'enabled': enabled_}
        if name_ == self.module:
            self.module_models['self'] = model
        else:
            self.module_models['children'].append(model)

    def _is_any_top_res_match(self, line):
        for re_pair in self.top_res:
            if re_pair[0].search(line):
                return True
        return False

    def _block_top_match(self, current_index, parent_item):
        line = self.lines[current_index]
        for re_pair in self.top_res:
            match_obj = re_pair[0].search(line)
            if match_obj:
                func = re_pair[1]
                if func:
                    return func(match_obj, current_index, parent_item)
        return 0

    def _parse_config(self, match_obj, start, parent_item):
        config_item = match_obj.group(1)
        parts = config_item.split(self.config_splitter)
        valid_parts = [part for part in parts if part]
        if not valid_parts:
            return
        config_name = valid_parts[-1].lower()
        depends_on_item = ''
        end = start + 1
        while end < len(self.lines):
            line = self.lines[end]
            match_obj = self.depends_on_re.search(line)
            if match_obj:
                depends_on_item = match_obj.group(1)
                break
            if self._is_any_top_res_match(line):
                break
            end += 1
        if not depends_on_item:
            depends_on_item = parent_item
            block_lines = end - start
        else:
            block_lines = end - start + 1
        enabled = self.dot_config.is_enabled(config_item, depends_on_item)
        self._add_model(config_name, config_item, depends_on_item, enabled)
        return block_lines

    def _parse_choice(self, _match_obj, start, _parent_item):
        end = start + 1
        common_depends_on_item = ''
        depends_on_obj = None
        while end < len(self.lines):
            line = self.lines[end]
            match_obj = self.depends_on_re.search(line)
            if match_obj:
                if not depends_on_obj:
                    depends_on_obj = match_obj
                    common_depends_on_item = match_obj.group(1)
                end += 1
                continue
            if self.endchoice_re.search(line):
                end += 1
                return end - start + 1
            consumed = self._block_top_match(end, common_depends_on_item)
            if consumed:
                end += consumed
            else:
                end += 1

    def get_models(self):
        if not os.path.exists(self.k_path):
            return
        self.lines = hdf_utils.read_file_lines(self.k_path)
        dot_config_path = hdf_utils.get_liteos_a_dot_config_path(self.root)
        self.dot_config = HdfDotConfigFile(dot_config_path)
        index = 0
        while index < len(self.lines):
            consume = self._block_top_match(index, '')
            if consume:
                index += consume
            else:
                index += 1
        return self.module_models

    def _get_driver_kconfig_item(self, driver):
        templates_dir = hdf_utils.get_templates_lite_dir()
        template = os.path.join(templates_dir, 'hdf_driver_kconfig.template')
        template_str = hdf_utils.read_file(template)
        mod_converter = hdf_utils.WordsConverter(self.module)
        drv_converter = hdf_utils.WordsConverter(driver)
        data_model = {
            'driver_upper_case': drv_converter.upper_case(),
            'driver_lower_case': drv_converter.lower_case(),
            'module_upper_case': mod_converter.upper_case()
        }
        config_item = 'DRIVERS_HDF_%s' % drv_converter.upper_case()
        depends_on_item = 'DRIVERS_HDF_%s' % mod_converter.upper_case()
        config_option = {'name': drv_converter.lower_case(),
                         'config_item': config_item,
                         'depends_on_item': depends_on_item,
                         'enabled': False}
        config = Template(template_str).safe_substitute(data_model)
        return config_option, config

    def _get_begin_end_flag(self, driver):
        id_ = hdf_utils.get_id(self.module, driver)
        begin_flag = '\n# <begin %s\n' % id_
        end_flag = '\n# %s end>\n' % id_
        return begin_flag, end_flag

    def add_driver(self, driver):
        file_content = hdf_utils.read_file(self.k_path)
        begin_flag, end_flag = self._get_begin_end_flag(driver)
        k_option, k_item = self._get_driver_kconfig_item(driver)
        new_content = hdf_utils.SectionContent(begin_flag, k_item, end_flag)
        old_content = hdf_utils.SectionContent(begin_flag, '', end_flag)
        old_range = hdf_utils.find_section(file_content, old_content)
        if old_range:
            hdf_utils.replace_and_save(file_content, self.k_path,
                                       old_range, new_content)
        else:
            hdf_utils.append_and_save(file_content, self.k_path, new_content)
        return k_option

    def delete_driver(self, driver):
        if not os.path.exists(self.k_path):
            return
        file_content = hdf_utils.read_file(self.k_path)
        begin_flag, end_flag = self._get_begin_end_flag(driver)
        old_content = hdf_utils.SectionContent(begin_flag, '', end_flag)
        old_range = hdf_utils.find_section(file_content, old_content)
        if not old_range:
            return
        hdf_utils.delete_and_save(file_content, self.k_path, old_range)
