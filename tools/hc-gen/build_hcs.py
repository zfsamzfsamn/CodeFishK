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
import sys
import argparse
import platform
import subprocess
import time


def exec_command(cmd):
    process = subprocess.Popen(cmd)
    process.wait()
    ret_code = process.returncode

    if ret_code != 0:
        raise Exception("{} failed, return code is {}".format(cmd, ret_code))


def make_hc_gen(current_dir):
    exec_command(['make', '-C', current_dir, '-j8'])


def prepare(current_dir):
    make_hc_gen(current_dir)


def main(argv):
    current_dir = os.path.split(os.path.realpath(__file__))[0]
    hc_gen = os.path.join(current_dir, 'build', 'hc-gen')

    build_hcs_cmd = [hc_gen] + argv[1:]

    prepare(current_dir)
    exec_command(build_hcs_cmd)


if __name__ == '__main__':
    sys.exit(main(sys.argv))
