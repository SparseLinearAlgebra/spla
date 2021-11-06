import shared
import argparse
import subprocess
import re


def test_names(test_root_path):
    cmake_file_path = test_root_path / "CMakeLists.txt"
    with open(cmake_file_path) as cmake_file:
        all_tests = re.findall(r"spla_test_target\((\w+)\)", cmake_file.read())
    return all_tests


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", default="build")
    args = parser.parse_args()

    tests_dir = shared.ROOT / args.build_dir / "tests"
    print(f"Searching for unit-tests in `{tests_dir}` folder")

    failed_tests = []
    all_tests = test_names(shared.ROOT / "tests")
    for test_name in all_tests:
        full_test_name = str(tests_dir / test_name)
        print(f"Exec unit-test: `{full_test_name}`")
        try:
            subprocess.check_call(full_test_name)
        except subprocess.CalledProcessError as err:
            failed_tests.append(test_name)
            print(f"Failed: `{err.output}`")

    all_tests_string = '\n\t'.join(all_tests)
    print(f"All executed tests: \n\t{all_tests_string}")

    if failed_tests:
        failed_tests_string = '\n\t'.join(failed_tests)
        raise Exception(f"Some tests have not been passed: \n\t{failed_tests_string}")


if __name__ == "__main__":
    main()
