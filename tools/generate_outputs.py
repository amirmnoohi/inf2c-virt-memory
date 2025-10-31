#!/usr/bin/env python3
"""
Generate output.txt for all test cases
Copyright (c) 2025 Amir Noohi
"""

import subprocess
import os
import sys

def parse_params(params_file):
    """Parse params.txt into command-line arguments"""
    args = []
    try:
        with open(params_file, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if line and '-' in line:
                    parts = line.split(' - ')
                    if len(parts) == 2:
                        param = parts[0].strip()
                        value = parts[1].strip()
                        args.extend([f'-{param}', value])
    except:
        return None
    return args

def generate(sim, testcase_dir):
    """Generate output for one test case"""
    name = os.path.basename(testcase_dir)
    input_file = os.path.join(testcase_dir, 'input.txt')
    params_file = os.path.join(testcase_dir, 'params.txt')
    output_file = os.path.join(testcase_dir, 'output.txt')
    
    if not os.path.exists(input_file) or not os.path.exists(params_file):
        return None
    
    args = parse_params(params_file)
    if not args:
        return None
    
    args.extend(['-t', input_file, '-v'])
    
    try:
        result = subprocess.run(
            [sim] + args,
            capture_output=True,
            text=True,
            encoding='utf-8',
            timeout=30
        )
        
        # Write output (stdout for valid, stderr for invalid configs)
        output_text = result.stdout if result.stdout else result.stderr
        
        with open(output_file, 'w', encoding='utf-8', newline='\n') as f:
            f.write(output_text)
        
        if result.returncode != 0:
            print(f"[OK] {name} (invalid config)")
        else:
            print(f"[OK] {name}")
        
        return True
        
    except Exception as e:
        print(f"[ERR] {name}: {e}")
        return False

def main():
    # Find simulator
    sim = None
    for candidate in ['./sim.exe', './sim', 'sim.exe', 'sim']:
        if os.path.exists(candidate):
            sim = candidate
            break
    
    if not sim:
        print("Error: Simulator not found. Run 'make' first.")
        sys.exit(1)
    
    print(f"Using simulator: {sim}\n")
    
    # Find all test cases
    testcases = sorted([
        os.path.join('tests', d)
        for d in os.listdir('tests')
        if d.startswith('testcase') and os.path.isdir(os.path.join('tests', d))
    ])
    
    count = 0
    for tc in testcases:
        result = generate(sim, tc)
        if result:
            count += 1
    
    print(f"\nGenerated: {count}/{len(testcases)}")

if __name__ == "__main__":
    main()
