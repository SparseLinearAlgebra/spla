import shared
import argparse
import subprocess

TESTS = [
    "TestBasic",
    "TestExpression"
]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--build-dir', default="build")
    args = parser.parse_args()

    tests_dir = shared.ROOT / args.build_dir / "tests"
    print(f"Searching for unit-tests in `{tests_dir}` folder")

    for test in TESTS:
        test_path = str(tests_dir / test)
        print(f"Exec unit-test: `{test_path}`")
        subprocess.call([test_path])


if __name__ == "__main__":
    main()
