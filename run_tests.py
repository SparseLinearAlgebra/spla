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
import subprocess
import re
import os
import json


def test_names(test_src_path):
    cmake_file_path = test_src_path / "CMakeLists.txt"
    with open(cmake_file_path) as cmake_file:
        all_tests = re.findall(r"spla_test_target\((\w+)\)", cmake_file.read())
    return all_tests


def is_raw_opencl_test(test_src_path, test_name):
    # Raw OpenCL tests (SPLA_GTEST_MAIN_NO_SPLA) do not initialize the spla
    # library, so the spla configuration mechanism does not apply to them.
    # They select the device via SPLA_OPENCL_PLATFORM / SPLA_OPENCL_DEVICE.
    with open(test_src_path / f"{test_name}.cpp") as test_file:
        return "SPLA_GTEST_MAIN_NO_SPLA" in test_file.read()


def resolve_config(config_file, config_name, build_dir=None):
    # Resolves a named configuration from a JSON file, applying `extends` in
    # order: each next parent overrides the previous one, and the parameters
    # of the configuration itself override all parents.
    with open(config_file) as conf_file:
        configs = json.load(conf_file)

    # Parents not defined in the explicit file are looked up in the
    # configuration file shipped with the library (next to the library in
    # the build dir), mirroring the library's file-level merge semantics.
    shipped_configs = {}
    if build_dir:
        shipped_file = shared.ROOT / build_dir / "spla_conf.json"
        if shipped_file.exists():
            with open(shipped_file) as shipped_conf:
                shipped_configs = json.load(shipped_conf)

    def resolve(name, stack):
        if name in stack:
            raise ValueError(f"extends cycle detected at configuration '{name}'")
        if name in configs:
            entry = dict(configs[name])
        elif name in shipped_configs:
            entry = dict(shipped_configs[name])
        else:
            raise ValueError(f"configuration '{name}' not found in {config_file}")

        extends = entry.pop("extends", [])
        resolved = {}
        for parent in extends:
            resolved.update(resolve(parent, stack + (name,)))
        resolved.update(entry)
        return resolved

    return resolve(config_name, ())


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", default="build")
    parser.add_argument("--config-file", required=True,
                        help="Path to the spla configuration file (passed to tests as --spla-config-file)")
    parser.add_argument("--config", required=True,
                        help="Name of the configuration to apply (passed to tests as --spla-config)")
    args = parser.parse_args()

    config = resolve_config(args.config_file, args.config, args.build_dir)
    backend = config.get("backend")

    # Raw OpenCL tests select the device via environment variables.
    spla_opencl_platform = "SPLA_OPENCL_PLATFORM"
    spla_opencl_device = "SPLA_OPENCL_DEVICE"
    run_raw_opencl_tests = True
    if backend == "by_index":
        os.environ[spla_opencl_platform] = str(config["platform_index"])
        os.environ[spla_opencl_device] = str(config["device_index"])
    elif backend == "cpu":
        # Pure CPU configuration: no OpenCL device is involved.
        run_raw_opencl_tests = False
    else:
        # opencl_gpu / opencl_cpu: no explicit indexes, keep the default (0/0).
        os.environ.setdefault(spla_opencl_platform, "0")
        os.environ.setdefault(spla_opencl_device, "0")

    tests_dir = shared.ROOT / args.build_dir / "tests"
    print(f"Searching for unit-tests in `{tests_dir}` folder")
    print(f"Using spla configuration '{args.config}' from `{args.config_file}` (backend: {backend})")

    failed_tests = []
    skipped_tests = []
    all_tests = test_names(shared.ROOT / "tests")
    for test_name in all_tests:
        if not run_raw_opencl_tests and is_raw_opencl_test(shared.ROOT / "tests", test_name):
            skipped_tests.append(test_name)
            print(f"Skipped: `{test_name}` (raw OpenCL test, backend=cpu)")
            continue

        full_test_name = str(tests_dir / test_name)
        cmd = [full_test_name,
               f"--spla-config-file={args.config_file}",
               f"--spla-config={args.config}"]
        try:
            subprocess.check_call(cmd)
            print(f"Exec unit-test: `{full_test_name}`")
        except FileNotFoundError:
            skipped_tests.append(test_name)
            print(f"Skipped: `{full_test_name}`")
        except subprocess.CalledProcessError as err:
            failed_tests.append(test_name)
            print(f"Failed: `{err.output}`")

    all_tests_string = '\n\t'.join(all_tests)
    print(f"All executed tests: \n\t{all_tests_string}")

    if skipped_tests:
        skipped_tests_string = '\n\t'.join(skipped_tests)
        print(f"Skipped tests: \n\t{skipped_tests_string}")

    if failed_tests:
        failed_tests_string = '\n\t'.join(failed_tests)
        raise Exception(f"Some tests have not been passed: \n\t{failed_tests_string}")


if __name__ == "__main__":
    main()
