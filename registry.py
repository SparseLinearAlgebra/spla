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

import argparse


def make_signature(ops, platform, arity, algo, t):
    return f"g_registry->add(MAKE_KEY_{platform.upper()}_{arity}(\"{algo}\"{ops}), " \
           f"std::make_shared<Algo_{algo}_{platform.lower()}<T_{t}>>());\n"


def make_ops_list(ops, t):
    return "{" + ",".join([f"{op}_{t}" for op in ops]) + "}"


def generate(file, algo, platform, ts, ops):
    arity = len(ops)

    for t in ts:
        for i, op in enumerate(ops):
            file.write(f"for (const auto& op{i}:" + make_ops_list(op, t) + ") {")

        key = f",{t}" if arity == 0 else "".join([f",op{i}" for i in range(arity)])
        file.write(make_signature(key, platform, arity, algo, t))

        for i in range(arity):
            file.write("}")


def main():
    parser = argparse.ArgumentParser("generate all possible algorithm variation")
    parser.add_argument("--out", help="file to save generated code", default="registry.txt")
    parser.add_argument("--full", help="gen full signature or short", default=False)
    parser.add_argument("--platform", help="target backend platform to generate", default="cpu")
    args = parser.parse_args()

    ts = ["INT", "UINT", "FLOAT"]
    ts_integral = ["INT", "UINT"]
    ops_bin = ["PLUS", "MINUS", "MULT", "DIV", "FIRST", "SECOND", "ONE", "MIN", "MAX"]
    ops_bin_x = ["BOR", "BAND", "BXOR"]
    ops_select = ["EQZERO", "NQZERO", "GTZERO", "GEZERO", "LTZERO", "LEZERO", "ALWAYS", "NEVER"]
    algos_0 = ["v_count_mf"]
    algos_1 = ["v_reduce", "v_eadd_fdb"]
    algos_2 = ["v_assign_masked"]
    algos_3 = ["mxv_masked", "vxm_masked"]
    algos_all = algos_0 + algos_1 + algos_2 + algos_3

    p = args.platform

    if not args.full:
        with open(args.out, "w") as file:
            for algo in algos_all:
                file.write(f"// algorthm {algo}\n")
                generate(file, algo, p, ts, [])
                file.write("\n")
    else:
        with open(args.out, "w") as file:
            for algo in algos_0:
                file.write(f"// algorthm {algo}\n")
                generate(file, algo, p, ts, [])
                file.write("\n\n")
            for algo in algos_1:
                file.write(f"// algorthm {algo}\n")
                generate(file, algo, p, ts, [ops_bin])
                generate(file, algo, p, ts_integral, [ops_bin_x])
                file.write("\n\n")
            for algo in algos_2:
                file.write(f"// algorthm {algo}\n")
                generate(file, algo, p, ts, [ops_bin, ops_select])
                generate(file, algo, p, ts_integral, [ops_bin_x, ops_select])
                file.write("\n\n")
            for algo in algos_3:
                file.write(f"// algorthm {algo}\n")
                generate(file, algo, p, ts, [ops_bin, ops_bin, ops_select])
                generate(file, algo, p, ts_integral, [ops_bin_x, ops_bin_x, ops_select])
                file.write("\n\n")


if __name__ == '__main__':
    main()
