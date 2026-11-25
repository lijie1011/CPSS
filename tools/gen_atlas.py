#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
姿态图集生成器：解析 OBJ 模型 → 渲染多姿态 2D 图集（Billboard Impostor）
用法: python gen_atlas.py <obj_dir> <output_dir> [size] [heading_steps] [roll_steps]
"""
import sys, os, math
import numpy as np
from PIL import Image, ImageDraw

HEADING_STEPS = 36   # 每10度
ROLL_STEPS = 7       # -45..45 每15度
TILE = 128           # 每张图 128x128


def load_obj(path):
    """解析 OBJ，返回 vertices(n,3), faces(list of vertex index list)"""
    verts, faces = [], []
    with open(path, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            if line.startswith('v '):
                parts = line.split()[1:4]
                if len(parts) >= 3:
                    verts.append([float(p) for p in parts])
            elif line.startswith('f '):
                idx = []
                for tok in line.split()[1:]:
                    v = tok.split('/')[0]
                    try:
                        i = int(v) - 1
                        if 0 <= i < len(verts): idx.append(i)
                    except ValueError:
                        pass
                if len(idx) >= 3:
                    faces.append(idx[:4])  # 最多四边形
    return np.array(verts, dtype=float), faces


def rot_x(p, a):
    c, s = math.cos(a), math.sin(a)
    return np.array([p[0], p[1]*c - p[2]*s, p[1]*s + p[2]*c])

def rot_y(p, a):
    c, s = math.cos(a), math.sin(a)
    return np.array([p[0]*c + p[2]*s, p[1], -p[0]*s + p[2]*c])

def rot_z(p, a):
    c, s = math.cos(a), math.sin(a)
    return np.array([p[0]*c - p[1]*s, p[0]*s + p[1]*c, p[2]])


def normalize(verts):
    """居中 + 缩放到统一尺寸"""
    if len(verts) == 0: return verts
    center = verts.mean(axis=0)
    v = verts - center
    span = np.max(np.linalg.norm(v, axis=1))
    if span > 0: v = v / span
    return v


def render_pose(verts, faces, heading_deg, roll_deg, tilt_deg=20, size=TILE):
    """渲染单个姿态，返回 PIL Image"""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    if len(verts) == 0: return img

    hd = math.radians(heading_deg)
    rl = math.radians(roll_deg)
    tl = math.radians(tilt_deg)

    scale = size * 0.62   # 放大填充
    cx = cy = size / 2

    def project(p):
        q = rot_y(p, rl)       # 横滚
        q = rot_z(q, -hd + math.pi/2)  # 航向
        q = rot_x(q, tl)       # 相机倾斜
        return (cx + q[0]*scale, cy - q[1]*scale)

    projected = [project(v) for v in verts]

    # 按深度排序（远到近）
    def depth(face):
        return np.mean([verts[i][2] for i in face])
    ordered = sorted(faces, key=depth)

    for face in ordered:
        pts = [projected[i] for i in face]
        # 简单光照：法线 Z 分量
        if len(face) >= 3:
            v0, v1, v2 = verts[face[0]], verts[face[1]], verts[face[2]]
            n = np.cross(v1-v0, v2-v0)
            nl = np.linalg.norm(n)
            if nl > 0:
                shade = max(0.25, min(1.0, 0.5 + 0.5*abs(n[2])/nl))
            else:
                shade = 0.7
        else:
            shade = 0.7
        # 灰白机体系，按阵营可着色（这里输出灰度，运行时着色）
        c = int(200 * shade)
        draw.polygon(pts, fill=(c, c, c, 255))

    return img


def gen_atlas(obj_path, out_dir, size=TILE):
    verts, faces = load_obj(obj_path)
    if len(verts) == 0:
        print(f'  SKIP {obj_path}: no vertices')
        return False
    verts = normalize(verts)
    name = os.path.splitext(os.path.basename(obj_path))[0]
    os.makedirs(out_dir, exist_ok=True)

    # 生成一张大图集：行=roll档, 列=heading档
    atlas = Image.new('RGBA', (HEADING_STEPS*size, ROLL_STEPS*size), (0,0,0,0))
    rolls = np.linspace(-45, 45, ROLL_STEPS)
    for h in range(HEADING_STEPS):
        hd = h * 360 // HEADING_STEPS
        for r, roll in enumerate(rolls):
            img = render_pose(verts, faces, hd, roll, size=size)
            atlas.paste(img, (h*size, r*size))

    out = os.path.join(out_dir, name + '.png')
    atlas.save(out)
    print(f'  {name}: {len(verts)}v {len(faces)}f -> {out} ({atlas.width}x{atlas.height})')
    return True


def main():
    if len(sys.argv) < 3:
        print('usage: gen_atlas.py <obj_dir> <output_dir> [size]')
        sys.exit(1)
    src, dst = sys.argv[1], sys.argv[2]
    size = int(sys.argv[3]) if len(sys.argv) > 3 else TILE
    objs = [os.path.join(src, f) for f in os.listdir(src) if f.lower().endswith('.obj')]
    print(f'found {len(objs)} OBJ models')
    ok = 0
    for o in objs:
        if gen_atlas(o, dst, size): ok += 1
    print(f'done: {ok}/{len(objs)} atlases generated')

if __name__ == '__main__':
    main()
