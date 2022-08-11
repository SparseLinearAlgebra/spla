import subprocess
import argparse
import shared


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", default="build")
    parser.add_argument("--build-type", default="Release")
    parser.add_argument("--tests", default="YES")
    parser.add_argument("--examples", default="YES")
    parser.add_argument("--opencl", default="YES")
    parser.add_argument("--target", default="all")
    parser.add_argument("--nt", default="4")
    parser.add_argument("--arch")
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
