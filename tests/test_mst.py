#!/usr/bin/env python3
"""
Test script for MST algorithm with Spla
Compares computed MST weight with expected weight for various graphs
"""

import subprocess
import os
import sys
import tempfile

# Путь к исполняемому файлу MST
MST_EXEC = "./build/mst"

# Тестовые случаи: (имя, вершины, рёбра, ожидаемый вес MST)
TEST_CASES = [
    # 2 nodes
    ("2_nodes_line", 2, [(0,1,5)], 5),
    
    # 3 nodes line
    ("3_nodes_line", 3, [(0,1,1), (1,2,2)], 3),
    
    # 4 nodes line
    ("4_nodes_line", 4, [(0,1,1), (1,2,2), (2,3,3)], 6),
    
    # 5 nodes line
    ("5_nodes_line", 5, [(0,1,1), (1,2,2), (2,3,3), (3,4,4)], 10),
    
    # 5 nodes star (center at 0)
    ("5_nodes_star", 5, [(0,1,5), (0,2,4), (0,3,3), (0,4,2)], 14),
    
    # 5 nodes complete (K5)
    ("5_nodes_complete", 5, [
        (0,1,1), (0,2,2), (0,3,3), (0,4,4),
        (1,2,5), (1,3,6), (1,4,7),
        (2,3,8), (2,4,9), (3,4,10)
    ], 10),  # MST weight = 1+2+3+4 = 10
    
    # Two components (6 vertices, but graph is disconnected)
    ("two_components", 6, [
        (0,1,1), (1,2,2), (0,2,3),  # component 1
        (3,4,1), (4,5,2), (3,5,3),   # component 2
    ], 1+2+1+2),  # MST of each component
    
    # Cycle 6 nodes
    ("cycle_6", 6, [
        (0,1,1), (1,2,2), (2,3,3), (3,4,4), (4,5,5), (5,0,6)
    ], 1+2+3+4+5),  # Remove the heaviest edge (6)
    
    # K3,3 bipartite graph
    ("k33", 6, [
        (0,3,1), (0,4,2), (0,5,3),
        (1,3,4), (1,4,5), (1,5,6),
        (2,3,7), (2,4,8), (2,5,9)
    ], 1+2+3+4+7),  # 5 edges connecting all vertices
    
    # Simple triangle
    ("triangle", 3, [(0,1,1), (0,2,1), (1,2,1)], 2),
    
    # Simple square
    ("square", 4, [(0,1,1), (1,2,1), (2,3,1), (0,3,1)], 3),
    
    # Square with different weights
    ("square_weighted", 4, [
        (0,1,2), (1,2,1), (2,3,2), (0,3,1)
    ], 1+1+2),  # edges: 0-3(1), 1-2(1), 0-1(2) or 0-1(2), 1-2(1), 2-3(2)
    
    # Your test graph (7 vertices)
    ("graph_7", 7, [
        (0,1,7), (0,4,4), (1,2,11), (1,3,10), (1,4,9),
        (2,3,5), (3,4,15), (3,5,12), (3,6,8), (4,5,6), (5,6,13)
    ], 40),
]


def create_mtx_file(n, edges, filename):
    """Create a .mtx file from edge list"""
    with open(filename, 'w') as f:
        f.write("%%MatrixMarket matrix coordinate real symmetric\n")
        f.write(f"{n} {n} {len(edges)}\n")
        for u, v, w in edges:
            # Convert to 1-based indices for .mtx format
            f.write(f"{u+1} {v+1} {w}\n")


def run_test(name, n, edges, expected_weight):
    """Run MST on a graph and compare weight"""
    # Create temporary file for the graph
    with tempfile.NamedTemporaryFile(mode='w', suffix='.mtx', delete=False) as f:
        mtx_file = f.name
        create_mtx_file(n, edges, mtx_file)
    
    try:
        # Run MST executable
        cmd = [MST_EXEC, "--mtxpath", mtx_file, "--run-cpu=false", "--run-gpu=true", "--niters=1"]
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        # Parse output to find MST total weight
        output = result.stdout
        weight = None
        
        for line in output.split('\n'):
            if "CPU MST total weight:" in line:
                # Extract weight from "CPU MST total weight: 40"
                weight = float(line.split(':')[1].strip())
            elif "GPU MST total weight:" in line:
                weight = float(line.split(':')[1].strip())
        
        # Check if test passed
        if weight is None:
            print(f"❌ {name}: Failed to parse output")
            print(output)
            return False
        
        if abs(weight - expected_weight) < 1e-6:
            print(f"✅ {name}: {weight} == {expected_weight}")
            return True
        else:
            print(f"❌ {name}: {weight} != {expected_weight}")
            return False
            
    except Exception as e:
        print(f"❌ {name}: Exception - {e}")
        return False
    finally:
        # Clean up temp file
        os.unlink(mtx_file)


def main():
    print("=" * 60)
    print("MST Algorithm Tests")
    print("=" * 60)
    
    # Check if executable exists
    if not os.path.exists(MST_EXEC):
        print(f"Error: {MST_EXEC} not found. Run build first.")
        sys.exit(1)
    
    passed = 0
    failed = 0
    
    for name, n, edges, expected in TEST_CASES:
        print(f"\nTesting: {name} (n={n}, edges={len(edges)})")
        if run_test(name, n, edges, expected):
            passed += 1
        else:
            failed += 1
    
    print("\n" + "=" * 60)
    print(f"Results: {passed} passed, {failed} failed")
    print("=" * 60)
    
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())