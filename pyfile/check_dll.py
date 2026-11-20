"""检查插件 DLL 是否包含 Qt 插件元数据与 IPlugin 标识。

路径基于本脚本所在目录（工程根），不再写死旧路径 d:\\ZCHH\\CPSS。
默认检查 Release 配置，可用命令行参数指定：python check_dll.py Debug
"""
import os
import sys

def check_dll_exports(dll_path):
    print(f"\n=== {os.path.basename(dll_path)} ===")
    if not os.path.exists(dll_path):
        print("  FILE NOT FOUND")
        return
    size = os.path.getsize(dll_path)
    print(f"  size: {size} bytes")

    with open(dll_path, 'rb') as f:
        data = f.read()

    for needle in [b'qt_plugin_query_metadata', b'qt_plugin_instance',
                   b'com.cpss.plugin.IPlugin', b'Q_PLUGIN_METADATA']:
        pos = data.find(needle)
        if pos >= 0:
            print(f"  FOUND '{needle.decode(errors='replace')}' at offset {pos}")
        else:
            print(f"  MISSING '{needle.decode(errors='replace')}'")

config = sys.argv[1] if len(sys.argv) > 1 else "Release"
# 本脚本位于 <工程根>/pyfile/，工程根是其上一级目录
root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
base = os.path.join(root, "src", "build", "bin", config, "plugins")

print(f"检查目录: {base}")
for dll in ["chartplugin.dll", "helloplugin.dll", "sampleplugin.dll", "dataflowplugin.dll"]:
    check_dll_exports(os.path.join(base, dll))
