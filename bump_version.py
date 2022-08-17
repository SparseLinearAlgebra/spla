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
