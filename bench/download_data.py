##################################################################################
# This file is part of spla project                                              #
# https://github.com/SparseLinearAlgebra/spla                                    #
##################################################################################
# MIT License                                                                    #
#                                                                                #
# Copyright (c) 2023 SparseLinearAlgebra                                         #
#                                                                                #
# Permission is hereby granted, free of charge, to any person obtaining a copy   #
# of this software and associated documentation files (the "Software"), to deal #
# in the Software without restriction, including without limitation the rights  #
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

"""Download benchmark matrices from the SuiteSparse Matrix Collection.

Matrices are cached in a local data directory (default: ~/spla-bench-data,
override with --data-dir or the SPLA_BENCH_DATA environment variable) so that
subsequent runs do not require network access. Each dataset lists several
source URLs (canonical host first) which are tried in order; a dataset that
is missing locally and cannot be downloaded from any of them fails the
script with a non-zero exit code.
"""

import argparse
import json
import os
import shutil
import sys
import tarfile
import tempfile
import urllib.request
from pathlib import Path


def default_data_dir():
    return Path(os.environ.get("SPLA_BENCH_DATA", str(Path.home() / "spla-bench-data")))


def default_manifest():
    return Path(__file__).resolve().parent / "matrices.json"


def download(url, dest):
    request = urllib.request.Request(url, headers={"User-Agent": "spla-benchmark/1.0"})
    with urllib.request.urlopen(request, timeout=300) as response:
        with open(dest, "wb") as out:
            shutil.copyfileobj(response, out)


def extract_dataset(tar_path, data_dir):
    with tarfile.open(tar_path) as tf:
        members = [m for m in tf.getmembers() if m.isfile() and m.name.endswith(".mtx")]
        if len(members) != 1:
            raise RuntimeError(f"expected exactly one .mtx member in {tar_path}, found {len(members)}")
        tf.extract(members[0], path=data_dir, filter="data")
        return data_dir / Path(members[0].name).name


def fetch_dataset(entry, data_dir):
    name = entry["name"]
    urls = entry["urls"]
    target = data_dir / f"{name}.mtx"

    if target.exists() and target.stat().st_size > 0:
        print(f"cached {name}")
        return

    errors = []
    for url in urls:
        try:
            with tempfile.TemporaryDirectory() as tmp:
                tar_path = Path(tmp) / "download.tar.gz"
                download(url, tar_path)
                extracted = extract_dataset(tar_path, data_dir)

            if extracted.name != target.name:
                extracted.rename(target)

            if not target.exists() or target.stat().st_size == 0:
                raise RuntimeError(f"extracted {target} is missing or empty")

            print(f"downloaded {name} from {url}")
            return
        except Exception as e:
            errors.append(f"{url}: {e}")

    raise RuntimeError("; ".join(errors))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data-dir", default=None, help="local matrix cache directory (default: ~/spla-bench-data)")
    parser.add_argument("--manifest", default=None, help="path to matrices.json manifest")
    args = parser.parse_args()

    data_dir = Path(args.data_dir).expanduser() if args.data_dir else default_data_dir()
    manifest_path = Path(args.manifest) if args.manifest else default_manifest()

    with open(manifest_path) as f:
        entries = json.load(f)

    data_dir.mkdir(parents=True, exist_ok=True)

    failed = []
    for entry in entries:
        try:
            fetch_dataset(entry, data_dir)
        except Exception as e:
            print(f"error: {entry['name']}: {e}", file=sys.stderr)
            failed.append(entry["name"])

    if failed:
        print(f"error: no local copy and download failed for: {', '.join(failed)}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
