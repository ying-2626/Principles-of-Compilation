import os
import subprocess
import sys
import shutil

# 测试配置
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
        "test_dir": "dataset/parser/ll",
        "dot_file": "ll_tree.dot"
    },
    "lr": {
        "src": "LRparser/LRparserMain.cpp",
        "include": "LRparser",
        "exe": "LRparser/output/LRparser.exe",
        "test_dir": "dataset/parser/lr",
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
    print(f"正在编译 {name}...")
    output_dir = os.path.dirname(config["exe"])
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # 处理 Windows 路径以及可能存在的空格/编码问题
    cmd = ["g++", config["src"], "-I", config["include"], "-o", config["exe"]]
    
    try:
        # 优先使用 GBK 解析 Windows 编译输出
        result = subprocess.run(cmd, capture_output=True, text=True, encoding='gbk')
    except UnicodeDecodeError:
        # 如果失败则退回到 UTF-8
        result = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8', errors='ignore')

    if result.returncode != 0:
        print(f"编译 {name} 出错:")
        print(result.stderr)
        return False
    print(f"编译 {name} 成功")
    return True

def convert_dot_to_png(dot_path, png_path):
    # 检查系统中是否存在 dot 命令（Graphviz）
    if shutil.which("dot") is None:
        return False
    
    try:
        subprocess.run(["dot", "-Tpng", dot_path, "-o", png_path], check=True)
        return True
    except Exception as e:
        print(f"    Error converting DOT to PNG: {e}")
        return False

def run_tests(name, config):
    print(f"正在为 {name} 运行测试用例...")
    exe_path = os.path.abspath(config["exe"])
    test_dir = os.path.abspath(config["test_dir"])
    
    if not os.path.exists(test_dir):
        print(f"测试目录 {test_dir} 不存在")
        return

    for filename in os.listdir(test_dir):
        if name == "lexical" and not filename.endswith(".c"): continue
        if name == "ll" and "ll_" not in filename: continue
        if name == "lr" and "lr_" not in filename: continue
        if name == "semantic" and not filename.endswith(".txt"): continue
        
        filepath = os.path.join(test_dir, filename)
        if not os.path.isfile(filepath): continue
        if filename.endswith(".out") or filename.endswith(".dot") or filename.endswith(".png"): continue

        print(f"  正在测试 {filename}...")
        try:
            # 如存在旧的 DOT 文件，先删除
            if "dot_file" in config and os.path.exists(config["dot_file"]):
                os.remove(config["dot_file"])

            # 运行可执行文件，将输入文件作为参数传入
            result = subprocess.run([exe_path, filepath], capture_output=True, text=True, timeout=5, encoding='utf-8', errors='ignore')
            
            print(f"    返回码: {result.returncode}")
            
            # 保存输出结果到 .out 文件
            output_file = filepath + ".out"
            with open(output_file, "w", encoding='utf-8') as f:
                f.write(result.stdout)
                if result.stderr:
                    f.write("\nSTDERR:\n")
                    f.write(result.stderr)
            print(f"    输出结果已保存到 {output_file}")
            
            # 处理可能生成的 DOT 文件
            if "dot_file" in config and os.path.exists(config["dot_file"]):
                # 将 DOT 文件移动到对应测试目录，并重命名
                target_dot = filepath + ".dot"
                if os.path.exists(target_dot):
                    os.remove(target_dot)
                os.rename(config["dot_file"], target_dot)
                print(f"    已生成 DOT 文件: {target_dot}")
                
                # 尝试将 DOT 转换为 PNG 图片
                target_png = filepath + ".png"
                if convert_dot_to_png(target_dot, target_png):
                    print(f"    已生成 PNG 图片: {target_png}")
                else:
                    print("    （未找到 Graphviz 的 dot 命令，跳过 PNG 生成）")

        except subprocess.TimeoutExpired as e:
            print(f"    运行测试出错: 命令在 {e.timeout} 秒后超时")
            if e.stdout:
                print("    标准输出 STDOUT:", e.stdout)
            if e.stderr:
                print("    标准错误 STDERR:", e.stderr)
        except Exception as e:
            print(f"    运行测试出错: {e}")

def main():
    for name, config in PROJECTS.items():
        if compile_project(name, config):
            run_tests(name, config)
        print("-" * 50)

if __name__ == "__main__":
    main()
