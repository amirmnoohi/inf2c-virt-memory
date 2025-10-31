#!/usr/bin/env python3
"""
Test runner for VM/Cache Simulator
Automatically finds and runs all testcaseNN/ directories
Copyright (c) 2025 Amir Noohi
"""

import subprocess
import sys
import os
from pathlib import Path

# ANSI colors
class C:
    G = '\033[92m'  # Green
    R = '\033[91m'  # Red
    Y = '\033[93m'  # Yellow
    B = '\033[94m'  # Blue
    M = '\033[95m'  # Magenta
    C = '\033[96m'  # Cyan
    END = '\033[0m'
    BOLD = '\033[1m'

def parse_params(params_file):
    """Parse params.txt and return arguments list"""
    args = []
    with open(params_file, 'r') as f:
        for line in f:
            line = line.strip()
            if line and '-' in line:
                parts = line.split(' - ')
                if len(parts) == 2:
                    param = parts[0].strip()
                    value = parts[1].strip()
                    args.extend([f'-{param}', value])
    return args

def run_test(sim, testcase_dir):
    """Run a single test case"""
    testcase_name = os.path.basename(testcase_dir)
    input_file = os.path.join(testcase_dir, 'input.txt')
    params_file = os.path.join(testcase_dir, 'params.txt')
    output_file = os.path.join(testcase_dir, 'output.txt')
    
    if not os.path.exists(input_file):
        return None  # Skip if no input file
    
    if not os.path.exists(params_file) or not os.path.exists(output_file):
        return None  # Skip if incomplete test
    
    # Parse parameters
    args = parse_params(params_file)
    args.extend(['-t', input_file, '-v'])  # Always verbose for output comparison
    
    # Run simulator
    try:
        result = subprocess.run(
            [sim] + args,
            capture_output=True,
            text=True,
            encoding='utf-8',
            timeout=10
        )
        
        # Read expected output (try multiple encodings)
        expected = None
        for encoding in ['utf-8', 'utf-8-sig', 'latin-1', 'cp1252']:
            try:
                with open(output_file, 'r', encoding=encoding) as f:
                    expected = f.read()
                break
            except:
                continue
        
        if expected is None:
            print(f"{C.R}[FAIL]{C.END} {testcase_name}: Cannot read output file")
            return False
        
        # Check if this is an invalid configuration test case
        is_invalid_config = expected.strip() == "Invalid configuration"
        
        if result.returncode != 0:
            # For invalid config tests, check stderr
            if is_invalid_config and result.stderr.strip() == "Invalid configuration":
                print(f"{C.G}[PASS]{C.END} {testcase_name}")
                return True
            print(f"{C.R}[FAIL]{C.END} {testcase_name}: Crashed (exit {result.returncode})")
            return False
        
        # Compare
        if result.stdout.strip() == expected.strip():
            print(f"{C.G}[PASS]{C.END} {testcase_name}")
            return True
        else:
            print(f"{C.R}[FAIL]{C.END} {testcase_name}: Output mismatch")
            return False
            
    except subprocess.TimeoutExpired:
        print(f"{C.R}[FAIL]{C.END} {testcase_name}: Timeout")
        return False
    except Exception as e:
        print(f"{C.R}[FAIL]{C.END} {testcase_name}: {e}")
        return False

def main():
    print(f"\n{C.BOLD}{C.M}{'='*70}{C.END}")
    print(f"{C.BOLD}{C.M}{'VM/Cache Simulator Test Suite':^70}{C.END}")
    print(f"{C.BOLD}{C.M}{'='*70}{C.END}\n")
    
    # Find simulator (prefer .exe on Windows)
    sim = None
    for candidate in ['./sim.exe', 'sim.exe', './sim', 'sim']:
        if os.path.exists(candidate):
            sim = candidate
            break
    
    if not sim:
        print(f"{C.R}[ERROR]{C.END} Simulator not found. Run 'make' first.")
        sys.exit(1)
    
    print(f"{C.B}[INFO]{C.END} Using: {sim}\n")
    
    # Find all testcaseNN directories
    test_dir = 'tests'
    testcases = sorted([
        os.path.join(test_dir, d) 
        for d in os.listdir(test_dir)
        if d.startswith('testcase') and os.path.isdir(os.path.join(test_dir, d))
    ])
    
    if not testcases:
        print(f"{C.Y}[WARN]{C.END} No test cases found in {test_dir}/")
        sys.exit(0)
    
    print(f"{C.B}[INFO]{C.END} Found {len(testcases)} test cases\n")
    
    # Run all tests
    results = []
    for testcase in testcases:
        result = run_test(sim, testcase)
        if result is not None:
            results.append(result)
    
    # Summary
    print(f"\n{C.BOLD}{C.M}{'='*70}{C.END}")
    print(f"{C.BOLD}{C.M}{'Summary':^70}{C.END}")
    print(f"{C.BOLD}{C.M}{'='*70}{C.END}\n")
    
    passed = sum(results)
    total = len(results)
    
    if passed == total:
        print(f"{C.BOLD}{C.G}All tests passed! ({passed}/{total}){C.END}")
        print(f"{C.G}{'*' * 70}{C.END}\n")
        sys.exit(0)
    else:
        print(f"{C.BOLD}{C.R}Tests passed: {passed}/{total}{C.END}")
        print(f"{C.R}{'!' * 70}{C.END}\n")
        sys.exit(1)

if __name__ == "__main__":
    main()
