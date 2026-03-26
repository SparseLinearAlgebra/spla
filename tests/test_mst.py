import subprocess
import os
import sys
import re
from pathlib import Path

MST_EXEC = "./build/mst"
TEST_DIR = "./test_graphs"

EXPECTED = {
    "2_nodes_line": 5, "3_nodes_line": 3, "4_nodes_line": 6,
    "5_nodes_line": 10, "5_nodes_star": 14, "5_nodes_complete": 10,
    "two_components": 6, "cycle_6": 15, "k33": 17,
    "triangle": 2, "square": 3, "square_weighted": 4, "graph_7": 40,
}
def run_test(mtx_file):
    cmd = [MST_EXEC, "--mtxpath", mtx_file, "--run-gpu=true", "--niters=1"]
    result = subprocess.run(cmd, capture_output=True, text=True)
    
    weight = None
    time  = None
    for line in result.stdout.split('\n'):
        if "GPU MST total weight:" in line:
            weight = float(line.split(':')[1].strip())
        if "GPU time (ms):" in line:
                time = float(line.split(':')[1][:-2].strip())
    
    return weight, time
def main():
    if not os.path.exists(MST_EXEC):
        print(f"Error: {MST_EXEC} not found")
        sys.exit(1)
    
    passed = 0
    failed = 0
    
    print(f"{'Test':<20} {'Expected':<10} {'Got':<10} {'Result':<10} {'Time(ms)':<10}")
    print("-" * 60)
    
    for mtx_file in sorted(Path(TEST_DIR).glob("*.mtx")):
        name = mtx_file.stem
        expected = EXPECTED.get(name)
        if expected is None:
            continue

        weight, time = run_test(str(mtx_file))
        
        if weight is None:
            print(f"{name:<20} {expected:<10} {'N/A':<10} {'❌ FAIL':<10} {time:.2f}")
            failed += 1
        elif abs(weight - expected) < 1e-6:
            print(f"{name:<20} {expected:<10} {weight:<10.1f} {'✅ PASS':<10} {time:.2f}")
            passed += 1
        else:
            print(f"{name:<20} {expected:<10} {weight:<10.1f} {'❌ FAIL':<10} {time:.2f}")
            failed += 1
    
    print("-" * 60)
    print(f"Results: {passed} passed, {failed} failed")


if __name__ == "__main__":
    main()