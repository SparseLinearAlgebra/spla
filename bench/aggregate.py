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

"""Aggregate per-run benchmark entries into one entry per algorithm per GPU.

Reads the JSON files written by the example applications with --bench (one
per algorithm x dataset) and produces a JSON array with one entry per
algorithm: the value is the sum of the per-dataset mean times and the extra
field keeps the per-dataset breakdown for chart tooltips. The output matches
the Continuous Benchmark action's customSmallerIsBetter schema.
"""

import argparse
import json
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--gpu", required=True, help="GPU label appended to each benchmark name")
    parser.add_argument("--out", required=True, help="path to write the aggregated json array")
    parser.add_argument("runs", nargs="+", help="per-run json files (one per algorithm x dataset)")
    args = parser.parse_args()

    groups = {}
    for run_path in args.runs:
        try:
            with open(run_path) as f:
                entry = json.load(f)
        except (OSError, json.JSONDecodeError) as e:
            print(f"error: cannot read {run_path}: {e}", file=sys.stderr)
            sys.exit(1)

        groups.setdefault(entry["name"], []).append((entry.get("dataset", "unknown"), entry["value"]))

    result = []
    for algo, runs in groups.items():
        extra = "; ".join(f"{dataset}: {value:.3f} ms" for dataset, value in runs)
        result.append(
            {
                "name": f"{algo} - {args.gpu}",
                "unit": "ms",
                "value": sum(value for _, value in runs),
                "extra": extra,
            }
        )

    with open(args.out, "w") as f:
        json.dump(result, f, indent=2)
        f.write("\n")


if __name__ == "__main__":
    main()
