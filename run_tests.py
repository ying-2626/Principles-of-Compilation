import os
import subprocess
import sys
import shutil

# Configuration
PROJECTS = {
    "lexical": {
        "src": "LexicalAnalysis/main.cpp",
        "include": "LexicalAnalysis",
        "exe": "LexicalAnalysis/output/main.exe",
        "test_dir": "dataset/lexical"
    },
    "ll": {
        "src": "LLparser/LLparserMain.cpp",
        "include": "LLparser",
        "exe": "LLparser/output/LLparser.exe",
        "test_dir": "dataset/parser",
        "dot_file": "ll_tree.dot"
    },
    "lr": {
        "src": "LRparser/LRparserMain.cpp",
        "include": "LRparser",
        "exe": "LRparser/output/LRparser.exe",
        "test_dir": "dataset/parser",
        "dot_file": "lr_tree.dot"
    },
    "semantic": {
        "src": "SemanticAnalysis/TranslationSchema.cpp",
        "include": "SemanticAnalysis",
        "exe": "SemanticAnalysis/main.exe",
        "test_dir": "dataset/semantic"
    }
}

def compile_project(name, config):
    print(f"Compiling {name}...")
    output_dir = os.path.dirname(config["exe"])
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # Handle Windows paths and potential spaces/encoding
    cmd = ["g++", config["src"], "-I", config["include"], "-o", config["exe"]]
    
    try:
        # Try GBK for Windows compilation output
        result = subprocess.run(cmd, capture_output=True, text=True, encoding='gbk')
    except UnicodeDecodeError:
        # Fallback to UTF-8
        result = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8', errors='ignore')

    if result.returncode != 0:
        print(f"Error compiling {name}:")
        print(result.stderr)
        return False
    print(f"Successfully compiled {name}")
    return True

def convert_dot_to_png(dot_path, png_path):
    # Check if dot command exists
    if shutil.which("dot") is None:
        return False
    
    try:
        subprocess.run(["dot", "-Tpng", dot_path, "-o", png_path], check=True)
        return True
    except Exception as e:
        print(f"    Error converting DOT to PNG: {e}")
        return False

def run_tests(name, config):
    print(f"Running tests for {name}...")
    exe_path = os.path.abspath(config["exe"])
    test_dir = os.path.abspath(config["test_dir"])
    
    if not os.path.exists(test_dir):
        print(f"Test directory {test_dir} does not exist")
        return

    for filename in os.listdir(test_dir):
        if name == "lexical" and not filename.endswith(".c"): continue
        if name == "ll" and "ll_" not in filename: continue
        if name == "lr" and "lr_" not in filename: continue
        if name == "semantic" and not filename.endswith(".txt"): continue
        
        filepath = os.path.join(test_dir, filename)
        if not os.path.isfile(filepath): continue
        if filename.endswith(".out") or filename.endswith(".dot") or filename.endswith(".png"): continue

        print(f"  Testing {filename}...")
        try:
            # Clean up previous DOT file if exists
            if "dot_file" in config and os.path.exists(config["dot_file"]):
                os.remove(config["dot_file"])

            # Run the executable with the input file as argument
            result = subprocess.run([exe_path, filepath], capture_output=True, text=True, timeout=5, encoding='utf-8', errors='ignore')
            
            print(f"    Return Code: {result.returncode}")
            
            # Save output
            output_file = filepath + ".out"
            with open(output_file, "w", encoding='utf-8') as f:
                f.write(result.stdout)
                if result.stderr:
                    f.write("\nSTDERR:\n")
                    f.write(result.stderr)
            print(f"    Output saved to {output_file}")
            
            # Handle DOT file
            if "dot_file" in config and os.path.exists(config["dot_file"]):
                # Move DOT file to test directory with specific name
                target_dot = filepath + ".dot"
                if os.path.exists(target_dot):
                    os.remove(target_dot)
                os.rename(config["dot_file"], target_dot)
                print(f"    DOT file generated: {target_dot}")
                
                # Try to convert to PNG
                target_png = filepath + ".png"
                if convert_dot_to_png(target_dot, target_png):
                    print(f"    PNG image generated: {target_png}")
                else:
                    print("    (Graphviz 'dot' command not found, skipping PNG generation)")

        except subprocess.TimeoutExpired as e:
            print(f"    Error running test: Command timed out after {e.timeout} seconds")
            if e.stdout:
                print("    STDOUT:", e.stdout)
            if e.stderr:
                print("    STDERR:", e.stderr)
        except Exception as e:
            print(f"    Error running test: {e}")

def main():
    for name, config in PROJECTS.items():
        if compile_project(name, config):
            run_tests(name, config)
        print("-" * 50)

if __name__ == "__main__":
    main()
