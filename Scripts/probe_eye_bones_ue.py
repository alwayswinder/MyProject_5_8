"""读取 UE 眼部相关骨骼世界坐标，与 Max 值对照（UE = Max.x, -Max.y, Max.z）。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
NAMES = ['Eye_R', 'Eye_L', 'Eye_R01', 'Eye_L01', 'yanqiu_R', 'yanqiu_L', 'yankuang_R', 'yankuang_L',
         'yandai_R', 'yandai_L', 'yandai2_R', 'yandai2_L', 'head001']

rig = unreal.load_asset(ASSET)
h = rig.get_hierarchy()

out = {}
for n in NAMES:
    k = unreal.RigElementKey(type=unreal.RigElementType.BONE, name=n)
    if not h.contains(k):
        out[n] = None
        continue
    t = h.get_global_transform(k)
    p = h.get_first_parent(k)
    out[n] = {
        't': [round(t.translation.x, 4), round(t.translation.y, 4), round(t.translation.z, 4)],
        'r': [round(t.rotation.x, 4), round(t.rotation.y, 4), round(t.rotation.z, 4), round(t.rotation.w, 4)],
        'parent': str(p.name) if p else None,
    }

print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
