#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
生成"演示页同款三角翼战斗机"姿态图集
顶点/面数据来自 aircraft_pose_demo.html，确保程序显示与演示页一致。
用法: python gen_demo_atlas.py <output_dir> [size]
"""
import sys, os, math
import numpy as np
from PIL import Image, ImageDraw

HEADING_STEPS = 36
ROLL_STEPS = 7

# ===== 演示页同款飞机模型 (本地坐标 +Y=机头, +X=右翼, +Z=上) =====
VERTS = np.array([
    [0, 6.0, 0],        # 0 机头
    [-0.45, 2.2, 0.25], # 1 前机身左上
    [0.45, 2.2, 0.25],  # 2 前机身右上
    [-0.45, 2.2, -0.25],# 3 前机身左下
    [0.45, 2.2, -0.25], # 4 前机身右下
    [-0.55, -4.0, 0.3], # 5 后机身左上
    [0.55, -4.0, 0.3],  # 6 后机身右上
    [-0.55, -4.0, -0.3],# 7 后机身左下
    [0.55, -4.0, -0.3], # 8 后机身右下
    [-4.2, -1.4, 0],    # 9 左翼尖前
    [-4.2, -2.3, 0],    # 10 左翼尖后
    [4.2, -1.4, 0],     # 11 右翼尖前
    [4.2, -2.3, 0],     # 12 右翼尖后
    [-2.0, -3.2, 0],    # 13 左尾翼尖前
    [-2.0, -3.9, 0],    # 14 左尾翼尖后
    [2.0, -3.2, 0],     # 15 右尾翼尖前
    [2.0, -3.9, 0],     # 16 右尾翼尖后
    [0, -2.4, 0.3],     # 17 垂尾前底
    [0, -4.0, 0.3],     # 18 垂尾后底
    [0, -3.7, 2.2],     # 19 垂尾顶
], dtype=float)

FACES = [
    ([0,1,2],    (232,232,232)), ([0,3,4],    (184,184,184)),  # 机头
    ([1,5,6,2],  (240,240,240)), ([3,4,8,7],  (160,160,160)),  # 机身上下
    ([1,3,7,5],  (200,200,200)), ([2,6,8,4],  (192,192,192)),  # 机身左右
    ([1,9,10,5], (220,220,220)), ([2,6,12,11],(208,208,208)),  # 主翼
    ([5,13,14],  (200,200,200)), ([6,16,15],  (196,196,196)),  # 尾翼
    ([17,19,18], (144,144,144)),                                # 垂尾
]


def rot_x(p, a):
    c, s = math.cos(a), math.sin(a)
    return np.array([p[0], p[1]*c - p[2]*s, p[1]*s + p[2]*c])
def rot_y(p, a):
    c, s = math.cos(a), math.sin(a)
    return np.array([p[0]*c + p[2]*s, p[1], -p[0]*s + p[2]*c])
def rot_z(p, a):
    c, s = math.cos(a), math.sin(a)
    return np.array([p[0]*c - p[1]*s, p[0]*s + p[1]*c, p[2]])


def normalize(v):
    center = v.mean(axis=0)
    v = v - center
    span = np.max(np.linalg.norm(v, axis=1))
    if span > 0: v = v / span
    return v


def render_pose(verts, heading_deg, roll_deg, tilt_deg=20, size=128):
    img = Image.new('RGBA', (size, size), (0,0,0,0))
    draw = ImageDraw.Draw(img)
    hd = math.radians(heading_deg)
    rl = math.radians(roll_deg)
    tl = math.radians(tilt_deg)
    scale = size * 0.42   # 略留边距
    cx = cy = size/2

    def project(p):
        q = rot_y(p, rl)
        q = rot_z(q, -hd + math.pi/2)
        q = rot_x(q, tl)
        return (cx + q[0]*scale, cy - q[1]*scale)

    projected = [project(v) for v in verts]
    # 深度排序（远到近）
    def depth(face):
        return np.mean([verts[i][2] for i in face[0]])
    ordered = sorted(FACES, key=depth)
    for face, color in ordered:
        pts = [projected[i] for i in face]
        draw.polygon(pts, fill=(color[0], color[1], color[2], 255))
    return img


def main():
    if len(sys.argv) < 2:
        print('usage: gen_demo_atlas.py <output_dir> [size]')
        sys.exit(1)
    out_dir = sys.argv[1]
    size = int(sys.argv[2]) if len(sys.argv)>2 else 128
    os.makedirs(out_dir, exist_ok=True)
    verts = normalize(VERTS)

    atlas = Image.new('RGBA', (HEADING_STEPS*size, ROLL_STEPS*size), (0,0,0,0))
    rolls = np.linspace(-45, 45, ROLL_STEPS)
    for h in range(HEADING_STEPS):
        hd = h * 360 // HEADING_STEPS
        for r, roll in enumerate(rolls):
            img = render_pose(verts, hd, roll, size=size)
            atlas.paste(img, (h*size, r*size))

    out = os.path.join(out_dir, 'demo_fighter.png')
    atlas.save(out)
    print(f'generated {out} ({atlas.width}x{atlas.height})')


if __name__ == '__main__':
    main()
