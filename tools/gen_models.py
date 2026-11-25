#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
生成 6 种程序化模型的姿态图集（复刻 aircraft_pose_demo.html）
每种模型 × 3 种边框色(gray/red/blue) → 18 张图集
用法: python gen_models.py <output_dir> [size]
"""
import sys, os, math
import numpy as np
from PIL import Image, ImageDraw

HEADING_STEPS = 36
ROLL_STEPS = 7
PITCH_STEPS = 5
PITCH_VALUES = [-30, -15, 0, 15, 30]

def hexrgb(h):
    h = h.lstrip('#')
    return (int(h[0:2],16), int(h[2:4],16), int(h[4:6],16))

class Builder:
    def __init__(self): self.V=[]; self.F=[]
    def addV(self, x,y,z): self.V.append([x,y,z]); return len(self.V)-1
    def addF(self, v, c): self.F.append((v, hexrgb(c)))
    def box(self, x0,x1,y0,y1,z0,z1,c):
        a=self.addV(x0,y0,z0); b=self.addV(x1,y0,z0); d=self.addV(x1,y1,z0); e=self.addV(x0,y1,z0)
        f=self.addV(x0,y0,z1); g=self.addV(x1,y0,z1); h=self.addV(x1,y1,z1); i=self.addV(x0,y1,z1)
        for face in ([a,b,d,e],[f,g,h,i],[a,b,g,f],[e,d,h,i],[a,e,i,f],[b,d,h,g]):
            self.addF(face, c)

def build_fighter():
    b=Builder()
    v=[b.addV(*p) for p in [
        [0,6,0],[-.45,2.2,.25],[.45,2.2,.25],[-.45,2.2,-.25],[.45,2.2,-.25],
        [-.55,-4,.3],[.55,-4,.3],[-.55,-4,-.3],[.55,-4,-.3],[-4.2,-1.4,0],[-4.2,-2.3,0],
        [4.2,-1.4,0],[4.2,-2.3,0],[-2,-3.2,0],[-2,-3.9,0],[2,-3.2,0],[2,-3.9,0],
        [0,-2.4,.3],[0,-4,.3],[0,-3.7,2.2]]]
    faces=[[0,1,2],[0,3,4],[1,5,6,2],[3,4,8,7],[1,3,7,5],[2,6,8,4],[1,9,10,5],[2,6,12,11],[5,13,14],[6,16,15],[17,19,18]]
    cols=['#e8e8e8','#b8b8b8','#f0f0f0','#a0a0a0','#c8c8c8','#c0c0c0','#dcdcdc','#d0d0d0','#c8c8c8','#c4c4c4','#909090']
    for f,c in zip(faces,cols): b.addF([v[k] for k in f], c)
    return b

def build_destroyer():
    b=Builder()
    bow=b.addV(0,7,0)
    bl=b.addV(-.9,5,.5); br=b.addV(.9,5,.5); bl2=b.addV(-.9,5,-.5); br2=b.addV(.9,5,-.5)
    sl=b.addV(-.9,-6,.5); sr=b.addV(.9,-6,.5); sl2=b.addV(-.9,-6,-.5); sr2=b.addV(.9,-6,-.5)
    b.addF([bow,bl,br],'#d8d8d8'); b.addF([bow,bl2,br2],'#909090')
    b.addF([bl,br,sr,sl],'#c8c8c8'); b.addF([bl2,br2,sr2,sl2],'#888888')
    b.addF([bl,bl2,sl2,sl],'#b0b0b0'); b.addF([br,br2,sr2,sr],'#b0b0b0')
    b.addF([sl,sr,sr2,sl2],'#787878')
    b.box(-.5,.5,2,4,.5,1.8,'#e0e0e0')
    b.box(-.3,.3,-2,0,.5,1.2,'#c0c0c0')
    b.box(-.15,.15,3.2,3.6,1.8,3.2,'#a0a0a0')
    return b

def build_carrier():
    b=Builder()
    bow=b.addV(0,8,0)
    bl=b.addV(-2.2,6,.8); br=b.addV(2.2,6,.8)
    sl=b.addV(-2.2,-8,.8); sr=b.addV(2.2,-8,.8)
    b.addF([bow,bl,br],'#8a8a8a'); b.addF([bl,br,sr,sl],'#9a9a9a')
    b.box(-2.2,2.2,-8,6,-.3,.8,'#606060')
    b.box(.8,2.0,2,4,.8,2.6,'#b0b0b0')
    b.box(1.1,1.7,2.4,2.8,2.6,4.0,'#909090')
    return b

def build_drone():
    b=Builder()
    nose=b.addV(0,3.5,0); tl=b.addV(-.3,2,.2); tr=b.addV(.3,2,.2)
    bl=b.addV(-.3,-2,.2); br=b.addV(.3,-2,.2)
    wl=b.addV(-3.5,.6,.05); wr=b.addV(3.5,.6,.05); wl2=b.addV(-3.5,-.6,.05); wr2=b.addV(3.5,-.6,.05)
    b.addF([nose,tl,tr],'#e0e0e0'); b.addF([tl,tr,br,bl],'#d0d0d0')
    b.addF([tl,wl,wl2,bl],'#e8e8e8'); b.addF([tr,br,wr2,wr],'#e8e8e8')
    b.addF([wl2,wr2],'#c0c0c0')
    t1=b.addV(-1.2,-1.8,.05); t2=b.addV(1.2,-1.8,.05)
    b.addF([bl,br,t2,t1],'#b8b8b8')
    return b

def build_recon():
    b=Builder()
    v=[b.addV(*p) for p in [
        [0,6,0],[-.4,3,.25],[.4,3,.25],[-.4,-4,.25],[.4,-4,.25],
        [-.4,3,-.25],[.4,3,-.25],[-.4,-4,-.25],[.4,-4,-.25],
        [-5,1.2,0],[5,1.2,0],[-5,-.8,0],[5,-.8,0],
        [-1.6,-3,.25],[1.6,-3,.25],[-1.6,-4,.25],[1.6,-4,.25],
        [0,-2.5,.25],[0,-4,.25],[0,-3.4,1.6]]]
    faces=[[0,1,2],[0,6,5],[1,3,4,2],[6,7,8,5],[1,5,7,3],[2,4,8,6],
           [1,9,10,5],[5,12,11,2],[3,13,14,4],[13,15,16,14],[17,19,18]]
    cols=['#e8e8e8','#b8b8b8','#f0f0f0','#a0a0a0','#c8c8c8','#c0c0c0','#dcdcdc','#c4c4c4','#c8c8c8','#bcbcbc','#909090']
    for f,c in zip(faces,cols): b.addF([v[k] for k in f], c)
    return b

def build_missile():
    b=Builder()
    nose=b.addV(0,6.5,0)
    rb=[b.addV(.35*math.cos(i*math.pi/4), 4.5, .35*math.sin(i*math.pi/4)) for i in range(8)]
    re=[b.addV(.35*math.cos(i*math.pi/4), -5, .35*math.sin(i*math.pi/4)) for i in range(8)]
    for i in range(8): b.addF([nose,rb[i],rb[(i+1)%8]], '#e8e8e8' if i%2 else '#d8d8d8')
    for i in range(8): b.addF([rb[i],rb[(i+1)%8],re[(i+1)%8],re[i]], '#c8c8c8' if i%2 else '#b8b8b8')
    b.addF(re, '#909090')
    return b

MODELS = {
    'fighter': build_fighter(),
    'destroyer': build_destroyer(),
    'carrier': build_carrier(),
    'drone': build_drone(),
    'recon': build_recon(),
    'missile': build_missile(),
}

def rot_x(p,a):
    c,s=math.cos(a),math.sin(a); return np.array([p[0],p[1]*c-p[2]*s,p[1]*s+p[2]*c])
def rot_y(p,a):
    c,s=math.cos(a),math.sin(a); return np.array([p[0]*c+p[2]*s,p[1],-p[0]*s+p[2]*c])
def rot_z(p,a):
    c,s=math.cos(a),math.sin(a); return np.array([p[0]*c-p[1]*s,p[0]*s+p[1]*c,p[2]])

def normalize(V):
    v=np.array(V,float); ctr=v.mean(axis=0); v=v-ctr
    span=np.max(np.linalg.norm(v,axis=1))
    if span>0: v=v/span
    return v

def render_pose(verts, faces, hd_deg, rl_deg, border, size, pt_deg=0, tilt=20, scale_k=0.42):
    img=Image.new('RGBA',(size,size),(0,0,0,0))
    draw=ImageDraw.Draw(img)
    hd=math.radians(hd_deg); rl=math.radians(rl_deg)
    pt=math.radians(pt_deg); tl=math.radians(tilt)
    scale=size*scale_k; cx=cy=size/2
    def proj(p):
        q=rot_y(p,rl)          # 横滚
        q=rot_x(q,-pt)         # 俯仰
        q=rot_z(q,-hd)         # 航向0=机头朝上(北)
        q=rot_x(q,tl)          # 相机倾斜
        return (cx+q[0]*scale, cy-q[1]*scale)
    pts=[proj(v) for v in verts]
    def depth(f): return np.mean([verts[i][2] for i in f[0]])
    for face,col in sorted(faces,key=depth):
        poly=[pts[i] for i in face]
        draw.polygon(poly, fill=(col[0],col[1],col[2],255))
        draw.line(poly+[poly[0]], fill=(border[0],border[1],border[2],255), width=2)
    return img

def gen_atlas(model_name, builder, border, out_dir, size):
    verts=normalize(builder.V)
    rows=ROLL_STEPS*PITCH_STEPS
    atlas=Image.new('RGBA',(HEADING_STEPS*size,rows*size),(0,0,0,0))
    rolls=np.linspace(-45,45,ROLL_STEPS)
    for h in range(HEADING_STEPS):
        hd=h*360//HEADING_STEPS
        for pi,pt in enumerate(PITCH_VALUES):
            for r,roll in enumerate(rolls):
                img=render_pose(verts,builder.F,hd,roll,border,size,pt_deg=pt)
                row=pi*ROLL_STEPS+r
                atlas.paste(img,(h*size,row*size))
    name=f"{model_name}_{'gray' if border==(64,64,64) else ('red' if border[0]>border[2] else 'blue')}"
    out=os.path.join(out_dir,name+'.png')
    atlas.save(out)
    return out

def main():
    if len(sys.argv)<2:
        print('usage: gen_models.py <output_dir> [size]'); sys.exit(1)
    out=sys.argv[1]; size=int(sys.argv[2]) if len(sys.argv)>2 else 128
    os.makedirs(out,exist_ok=True)
    borders={'gray':(64,64,64),'red':(255,68,68),'blue':(68,136,255)}
    for name,b in MODELS.items():
        for bname,border in borders.items():
            f=gen_atlas(name,b,border,out,size)
            print(f'{name}_{bname}: {os.path.getsize(f)//1024}KB')
    print(f'done: {len(MODELS)*3} atlases')

if __name__=='__main__': main()
