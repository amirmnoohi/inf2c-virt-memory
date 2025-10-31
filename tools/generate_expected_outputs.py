#!/usr/bin/env python3
"""
Generate expected output files for test cases
"""

import subprocess
import sys
import os

def generate_output(sim_path, args, output_file):
    """Generate expected output by running the simulator"""
    try:
        result = subprocess.run(
            [sim_path] + args,
            capture_output=True,
            text=True,
            encoding='utf-8',
            timeout=5
        )
        
        if result.returncode != 0:
            print(f"Error generating {output_file}: {result.stderr}")
            return False
        
        # Write output to file with UTF-8 encoding
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(result.stdout)
        
        print(f"[OK] Generated {output_file}")
        return True
        
    except Exception as e:
        print(f"[ERR] Failed to generate {output_file}: {e}")
        return False

def main():
    sim_path = "./sim.exe" if os.path.exists("./sim.exe") else "./sim"
    
    if not os.path.exists(sim_path):
        print("Error: Simulator not found. Run 'make' first.")
        sys.exit(1)
    
    print("Generating expected output files...\n")
    
    tests = [
        {
            "name": "Task 1",
            "args": ["-S", "1024", "-T", "4", "-L", "1", "-t", "tests/task1/trace.txt"],
            "output": "tests/task1/expected_output.txt"
        },
        {
            "name": "Task 2",
            "args": ["-S", "1024", "-B", "32", "-T", "4", "-L", "1", "-t", "tests/task2/trace.txt"],
            "output": "tests/task2/expected_output.txt"
        },
        {
            "name": "Task 3",
            "args": ["-S", "1024", "-B", "16", "-A", "3", "-T", "4", "-L", "1", "-t", "tests/task3/trace.txt"],
            "output": "tests/task3/expected_output.txt"
        },
        {
            "name": "Task 4",
            "args": ["-S1", "32768", "-B1", "64", "-A1", "2", 
                     "-S2", "262144", "-B2", "64", "-A2", "1",
                     "-T", "4", "-L", "1", "-t", "tests/task4/trace.txt"],
            "output": "tests/task4/expected_output.txt"
        }
    ]
    
    success = True
    for test in tests:
        if not generate_output(sim_path, test["args"], test["output"]):
            success = False
    
    if success:
        print("\n[SUCCESS] All expected outputs generated successfully!")
    else:
        print("\n[ERROR] Some outputs failed to generate")
        sys.exit(1)

if __name__ == "__main__":
    main()

