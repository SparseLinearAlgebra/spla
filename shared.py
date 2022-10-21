##################################################################################
# This file is part of spla project                                              #
# https://github.com/JetBrains-Research/spla                                     #
##################################################################################
# MIT License                                                                    #
#                                                                                #
# Copyright (c) 2021-2022 JetBrains-Research                                     #
#                                                                                #
# Permission is hereby granted, free of charge, to any person obtaining a copy   #
# of this software and associated documentation files (the "Software"), to deal  #
# in the Software without restriction, including without limitation the rights   #
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      #
# copies of the Software, and to permit persons to whom the Software is          #
# furnished to do so, subject to the following conditions:                       #
#                                                                                #
# The above copyright notice and this permission notice shall be included in all #
# copies or substantial portions of the Software.                                #
#                                                                                #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     #
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       #
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    #
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         #
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  #
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  #
# SOFTWARE.                                                                      #
##################################################################################

import pathlib
import platform

ROOT = pathlib.Path(__file__).parent
INCLUDE = ROOT / "include"
SOURCES = ROOT / "src"
PACKAGE = ROOT / "python"

ARCH = {'AMD64': 'x64', 'x86_64': 'x64', 'arm64': 'arm64'}[platform.machine()]
SYSTEM = {'Darwin': 'macos', 'Linux': 'linux', 'Windows': 'windows'}[platform.system()]
EXECUTABLE_EXT = {'macos': '', 'windows': '.exe', 'linux': ''}[SYSTEM]
TARGET_SUFFIX = {'macos': '.dylib', 'linux': '.so', 'windows': '.dll'}[SYSTEM]
TARGET = {'macos': 'libspla', 'linux': 'libspla', 'windows': 'spla'}[SYSTEM] + "_" + ARCH + TARGET_SUFFIX
