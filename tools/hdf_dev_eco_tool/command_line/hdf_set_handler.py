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
import json
import argparse

from .hdf_command_handler_base import HdfCommandHandlerBase
from .hdf_command_error_code import CommandErrorCode
from .hdf_dot_config_file import HdfDotConfigFile
from .hdf_model_enable_operation import EnableOperation
from .hdf_vendor_kconfig_file import HdfVendorKconfigFile
from .hdf_vendor_mk_file import HdfVendorMkFile
from .hdf_lite_mk_file import HdfLiteMkFile
from .hdf_lite_kconfig_file import HdfLiteKconfigFile
from hdf_tool_settings import HdfToolSettings
from hdf_tool_exception import HdfToolException
import hdf_utils


class ConfigItemsAction(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        setattr(namespace, self.dest, " ".join(values))


class HdfSetHandler(HdfCommandHandlerBase):
    def __init__(self, args):
        super(HdfSetHandler, self).__init__()
        self.cmd = 'set'
        self.handlers = {
            'current_vendor': self._set_current_vendor_handler,
            'current_board': self._set_current_board_handler,
            'vendor_new_name': self._set_vendor_new_name_handler,
            'drivers_state': self._set_drivers_state_handler,
            'model_enable': self._enable_operation,
            'model_disable': self._disable_operation,
        }
        self.parser.add_argument("--action_type",
                                 help=' '.join(self.handlers.keys()),
                                 required=True)
        self.parser.add_argument("--root_dir", required=True)
        self.parser.add_argument("--vendor_name")
        self.parser.add_argument("--board_name")
        self.parser.add_argument("--module_name")
        self.parser.add_argument("--new_vendor_name")
        self.parser.add_argument("--all_drivers", nargs='*',
                                 action=ConfigItemsAction)
        self.args = self.parser.parse_args(args)

    def _set_current_vendor_handler(self):
        self.check_arg_raise_if_not_exist("vendor_name")
        root, vendor, _, _, _ = self.get_args()
        HdfLiteMkFile(root).set_vendor(vendor)
        HdfLiteKconfigFile(root).set_vendor(vendor)

    def _set_current_board_handler(self):
        return

    def _set_vendor_new_name_handler(self):
        self.check_arg_raise_if_not_exist("vendor_name")
        self.check_arg_raise_if_not_exist("new_vendor_name")
        root_dir = self.args.root_dir
        old_name = self.args.vendor_name
        new_name = self.args.new_vendor_name
        src_vendor = hdf_utils.get_vendor_dir(root_dir, old_name)
        if not os.path.exists(src_vendor):
            raise HdfToolException('vendor: "%s" not exist' % old_name,
                                   CommandErrorCode.TARGET_NOT_EXIST)
        dst_vendor = hdf_utils.get_vendor_dir(root_dir, new_name)
        if os.path.exists(dst_vendor):
            raise HdfToolException('vendor: "%s" already exist' % new_name,
                                   CommandErrorCode.TARGET_ALREADY_EXIST)
        need_update_current_vendor = False
        hdf_lite = HdfLiteMkFile(root_dir)
        if hdf_lite.get_current_vendor() == old_name:
            need_update_current_vendor = True
        os.rename(src_vendor, dst_vendor)
        if need_update_current_vendor:
            hdf_lite.set_vendor(new_name)
        HdfVendorKconfigFile(root_dir, new_name).rename_vendor()
        HdfVendorMkFile(root_dir, new_name).rename_vendor()

    def _set_drivers_state_handler(self):
        self.check_arg_raise_if_not_exist("all_drivers")
        self.check_arg_raise_if_not_exist("board_name")
        drivers = json.loads(self.args.all_drivers)
        board_name = self.args.board_name.lower()
        root = self.args.root_dir
        dot_config_path = hdf_utils.get_liteos_a_dot_config_path(root)
        dot_config = HdfDotConfigFile(dot_config_path)
        orig_dot_configs = HdfToolSettings().get_dot_configs(board_name)
        orig_dot_configs = \
            [os.path.join(root, config) for config in orig_dot_configs]
        orig_dot_configs = \
            [HdfDotConfigFile(config) for config in orig_dot_configs]

        def config_set(config):
            if not config:
                return
            enabled = config.get('enabled', False)
            config_item = config.get('config_item', '')
            depends_on_item = config.get('depends_on_item', '')
            if enabled:
                dot_config.enable(config_item, depends_on_item)
                for orig_dot_config in orig_dot_configs:
                    orig_dot_config.enable(config_item, depends_on_item)
            else:
                dot_config.disable(config_item)
                for orig_dot_config in orig_dot_configs:
                    orig_dot_config.disable(config_item)

        for module, module_state in drivers.items():
            config_set(module_state.get('self', ''))
            for drv_config in module_state.get('children', []):
                config_set(drv_config)
        dot_config.save()
        for orig_dot_config_ in orig_dot_configs:
            orig_dot_config_.save()

    def _enable_operation(self):
        self.check_arg_raise_if_not_exist("root_dir")
        self.check_arg_raise_if_not_exist("vendor_name")
        self.check_arg_raise_if_not_exist("board_name")
        self.check_arg_raise_if_not_exist("module_name")
        root, vendor, model, _, board, _ = self.get_args()
        return EnableOperation(root=root, vendor=vendor, board=board, model=model).operation_enable()

    def _disable_operation(self):
        self.check_arg_raise_if_not_exist("root_dir")
        self.check_arg_raise_if_not_exist("vendor_name")
        self.check_arg_raise_if_not_exist("board_name")
        self.check_arg_raise_if_not_exist("module_name")
        root, vendor, model, _, board, _ = self.get_args()
        return EnableOperation(root=root, vendor=vendor, board=board, model=model).operation_disable()