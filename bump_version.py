##################################################################################
# This file is part of spla project                                              #
# https://github.com/SparseLinearAlgebra/spla                                    #
##################################################################################
# MIT License                                                                    #
#                                                                                #
# Copyright (c) 2023 SparseLinearAlgebra                                         #
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

import shared
import argparse
import fileinput


def inc_version(version):
    return [version[0], version[1], version[2] + 1]


def version_to_str(version):
    return f"{version[0]}.{version[1]}.{version[2]}"


def read_version():
    with open(shared.PACKAGE / "VERSION.md", "r") as f:
        return [int(v) for v in f.readline().split(".")]


def write_version(version):
    with open(shared.PACKAGE / "VERSION.md", "w") as f:
        f.write(version)
    with open(shared.PACKAGE / "pyspla" / "version.py", "a") as f:
        f.write(f"VERSIONS.append(\"{version}\")\n")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-inc", default=False, help="increments version of the package")
    parser.add_argument("--bump", help="bump specified version into package")
    args = parser.parse_args()

    if args.bump:
        write_version(args.bump)
        return
    if args.inc:
        version = read_version()
        new_version = inc_version(version)
        write_version(version_to_str(new_version))
        return

    print("no option specified: nothing to do")


if __name__ == '__main__':
    main()
