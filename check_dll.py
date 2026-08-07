import os, sys, struct

def check_dll_exports(dll_path):
    print(f"\n=== {os.path.basename(dll_path)} ===")
    if not os.path.exists(dll_path):
        print("  FILE NOT FOUND")
        return
    size = os.path.getsize(dll_path)
    print(f"  size: {size} bytes")
    
    with open(dll_path, 'rb') as f:
        data = f.read()
    
    # Search for Qt plugin metadata strings
    for needle in [b'qt_plugin_query_metadata', b'qt_plugin_instance', 
                   b'com.cpss.plugin.IPlugin', b'Q_PLUGIN_METADATA']:
        pos = data.find(needle)
        if pos >= 0:
            print(f"  FOUND '{needle.decode(errors='replace')}' at offset {pos}")
        else:
            print(f"  MISSING '{needle.decode(errors='replace')}'")

base = r"d:\ZCHH\CPSS\src\build\bin\Debug\plugins"
for dll in ["helloplugin.dll", "sampleplugin.dll", "dataflowplugin.dll"]:
    check_dll_exports(os.path.join(base, dll))
