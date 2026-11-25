#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
从 GeoServer WMTS 下载缺失瓦片（中国范围 z1~z8）
多线程并发，已存在的跳过。直接写入 mapdata/EPSG_4326/{z}/{x}/{y}.png
"""
import os, sys, math, time
import urllib.request
from concurrent.futures import ThreadPoolExecutor, as_completed

BASE = 'http://localhost:8082/geoserver/gwc/service/wmts'
DEST = r'D:\CPSS\src\3dParty\mapdata\EPSG_4326'
BBOX = (73.0, 18.0, 135.0, 54.0)   # lonMin, latMin, lonMax, latMax
ZOOMS = range(1, 9)
WORKERS = 16

def tile_range(z):
    n = 2 ** z
    x0 = int(math.floor((BBOX[0]+180)/360 * 2*n))
    x1 = int(math.floor((BBOX[2]+180)/360 * 2*n))
    y0 = int(math.floor((90-BBOX[3])/180 * n))
    y1 = int(math.floor((90-BBOX[1])/180 * n))
    return x0, x1, y0, y1

def url(z, x, y):
    return (f'{BASE}?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0'
            f'&LAYER=world_dom&TILEMATRIXSET=EPSG:4326'
            f'&TILEMATRIX=EPSG:4326:{z}&TILEROW={y}&TILECOL={x}'
            f'&FORMAT=image/png')

def download_one(task):
    z, x, y = task
    path = os.path.join(DEST, str(z), str(x), f'{y}.png')
    if os.path.exists(path) and os.path.getsize(path) > 100:
        return ('skip', 0)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    for retry in range(3):
        try:
            with urllib.request.urlopen(url(z,x,y), timeout=30) as resp:
                data = resp.read()
            if len(data) < 100:
                return ('small', 0)
            with open(path, 'wb') as f:
                f.write(data)
            return ('ok', len(data))
        except Exception as e:
            if retry == 2:
                return ('fail', str(e)[:80])
            time.sleep(0.5)

def main():
    tasks = []
    for z in ZOOMS:
        x0,x1,y0,y1 = tile_range(z)
        for x in range(x0, x1+1):
            for y in range(y0, y1+1):
                tasks.append((z,x,y))
    print(f'total expected tiles: {len(tasks)}')
    existing = sum(1 for z,x,y in tasks if os.path.exists(os.path.join(DEST,str(z),str(x),f'{y}.png')))
    print(f'already exists: {existing}, to download: {len(tasks)-existing}')

    stats = {'ok':0, 'skip':0, 'small':0, 'fail':0}
    t0 = time.time()
    with ThreadPoolExecutor(max_workers=WORKERS) as pool:
        futures = {pool.submit(download_one, t): t for t in tasks}
        for i, fut in enumerate(as_completed(futures), 1):
            status, val = fut.result()
            stats[status] = stats.get(status,0) + 1
            if i % 500 == 0:
                print(f'  progress {i}/{len(tasks)} ok={stats["ok"]} fail={stats["fail"]} elapsed={int(time.time()-t0)}s', flush=True)
            if status == 'fail' and stats['fail'] <= 5:
                print(f'  FAIL {futures[fut]}: {val}', flush=True)

    print(f'done in {int(time.time()-t0)}s: {stats}')

if __name__ == '__main__':
    main()
