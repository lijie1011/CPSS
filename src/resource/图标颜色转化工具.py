#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
图标转换脚本：将 JB/ 目录下的彩色 PNG 图标转为白色线稿模板，输出到 JB_Souce/
用法：python convert_icons.py

转换逻辑：
  1. 读取 JB/ 目录下所有 .png 文件
  2. 对每个像素：保留 alpha 通道，将 RGB 设为白色 (255,255,255)
  3. 输出到 JB_Souce/ 目录，保持相同文件名
"""

import os
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("错误: 需要安装 Pillow 库")
    print("运行: pip install Pillow")
    sys.exit(1)

# 路径配置
SCRIPT_DIR = Path(__file__).parent.absolute()
SRC_DIR = SCRIPT_DIR / "src" / "resource" / "JB"
DST_DIR = SCRIPT_DIR / "src" / "resource" / "JB_Souce"

def convert_icon(src_path: Path, dst_path: Path):
    """将单个图标转换为白色线稿模板"""
    img = Image.open(src_path).convert("RGBA")
    pixels = img.load()
    width, height = img.size

    for y in range(height):
        for x in range(width):
            r, g, b, a = pixels[x, y]
            if a > 0:
                # 有颜色的区域设为白色，保留 alpha
                pixels[x, y] = (255, 255, 255, a)
            # a == 0 的区域保持透明

    dst_path.parent.mkdir(parents=True, exist_ok=True)
    img.save(dst_path, "PNG")
    return True

def main():
    if not SRC_DIR.exists():
        print(f"错误: 源目录不存在: {SRC_DIR}")
        sys.exit(1)

    png_files = sorted(SRC_DIR.glob("*.png"))
    if not png_files:
        print(f"警告: {SRC_DIR} 下没有找到 PNG 文件")
        sys.exit(1)

    print(f"找到 {len(png_files)} 个图标文件")
    print(f"源目录: {SRC_DIR}")
    print(f"目标目录: {DST_DIR}")

    # 清空目标目录
    if DST_DIR.exists():
        for f in DST_DIR.glob("*.png"):
            f.unlink()
    DST_DIR.mkdir(parents=True, exist_ok=True)

    success = 0
    for png in png_files:
        dst = DST_DIR / png.name
        if convert_icon(png, dst):
            print(f"  OK  {png.name}")
            success += 1
        else:
            print(f"  FAIL  {png.name}")

    print(f"\n完成! 成功转换 {success}/{len(png_files)} 个图标")

if __name__ == "__main__":
    main()