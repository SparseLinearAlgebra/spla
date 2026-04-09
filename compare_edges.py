#!/usr/bin/env python3
import sys
import re

def parse_lagraph_edges(filename):
    """Парсит рёбра из вывода msf_demo в формате (0, 562)   -5.73066"""
    edges = {}
    with open(filename, 'r') as f:
        for line in f:
            # Удаляем пробелы в начале и конце
            line = line.strip()
            # Формат: (0, 562)   -5.73066
            match = re.match(r'\(\s*(\d+),\s*(\d+)\)\s+([-\d.]+)', line)
            if match:
                u, v, w = int(match.group(1)), int(match.group(2)), float(match.group(3))
                edges[(min(u, v), max(u, v))] = w
    return edges

def parse_spla_edges(filename):
    """Парсит рёбра из вывода mst в формате (0, 4) -9.01713"""
    edges = {}
    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            # Формат: (0, 4) -9.01713
            match = re.match(r'\(\s*(\d+),\s*(\d+)\)\s+([-\d.]+)', line)
            if match:
                u, v, w = int(match.group(1)), int(match.group(2)), float(match.group(3))
                edges[(min(u, v), max(u, v))] = w
    return edges

def compare_edges(lagraph_file, spla_file):
    print("Loading edges...")
    lagraph = parse_lagraph_edges(lagraph_file)
    spla = parse_spla_edges(spla_file)
    
    print(f"\nLAGraph edges: {len(lagraph)}")
    print(f"Spla edges: {len(spla)}")
    
    # Общие рёбра
    common = set(lagraph.keys()) & set(spla.keys())
    only_lagraph = set(lagraph.keys()) - set(spla.keys())
    only_spla = set(spla.keys()) - set(lagraph.keys())
    
    print(f"\nCommon edges: {len(common)}")
    print(f"Only in LAGraph: {len(only_lagraph)}")
    print(f"Only in Spla: {len(only_spla)}")
    
    # Вывод первых 10 различных рёбер
    if only_lagraph:
        print("\n--- First 10 edges only in LAGraph ---")
        for i, e in enumerate(sorted(only_lagraph)[:10]):
            print(f"  {e[0]}-{e[1]}: {lagraph[e]:.6f}")
    
    if only_spla:
        print("\n--- First 10 edges only in Spla ---")
        for i, e in enumerate(sorted(only_spla)[:10]):
            print(f"  {e[0]}-{e[1]}: {spla[e]:.6f}")
    
    # Общая сумма
    sum_lagraph = sum(lagraph.values())
    sum_spla = sum(spla.values())
    print(f"\nTotal weight LAGraph: {sum_lagraph:.6f}")
    print(f"Total weight Spla: {sum_spla:.6f}")
    print(f"Difference: {abs(sum_lagraph - sum_spla):.6f}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 compare_edges.py <lagraph_output.txt> <spla_output.txt>")
        sys.exit(1)
    compare_edges(sys.argv[1], sys.argv[2])