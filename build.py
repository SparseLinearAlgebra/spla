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

import subprocess
import argparse
import shared


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", default="build", help="folder name to locate build files")
    parser.add_argument("--build-type", default="Release", help="CMake type of build `Debug` or `Release`")
    parser.add_argument("--tests", default="YES", help="build tests")
    parser.add_argument("--examples", default="YES", help="build example applications")
    parser.add_argument("--opencl", default="YES", help="build opencl acceleration backend")
    parser.add_argument("--target", default="all", help="which target to build")
    parser.add_argument("--nt", default="4", help="number of os threads for build")
    parser.add_argument("--arch", default="target architecture on MacOS `x64` or `arm64`")
    args = parser.parse_args()

    build_config_args = ["cmake", ".", "-B", args.build_dir, "-G", "Ninja", f"-DCMAKE_BUILD_TYPE={args.build_type}",
                         f"-DSPLA_BUILD_TESTS={args.tests}", f"-DSPLA_BUILD_EXAMPLES={args.examples}",
                         f"-DSPLA_BUILD_OPENCL={args.opencl}"]

    if args.arch:
        build_config_args += [f"-DCMAKE_OSX_ARCHITECTURES={args.arch}"]

    build_run_args = ["cmake", "--build", args.build_dir, "--target", args.target, "--verbose", "-j", args.nt]

    subprocess.check_call(build_config_args)
    subprocess.check_call(build_run_args)


if __name__ == '__main__':
    main()
