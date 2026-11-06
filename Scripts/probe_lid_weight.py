"""实验：只平移 Bone028 向下 2cm（不改旋转），看眼皮顶点是否跟随，以判断权重归属。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def bkey(name):
    return unreal.RigElementKey(type=unreal.RigElementType.BONE, name=name)


rig = unreal.load_asset(ASSET)
inst = rig.get_debugged_control_rig()
out = {}
if inst is None:
    out['err'] = 'no debugged instance'
else:
    rh = inst.get_hierarchy()
    t = rh.get_global_transform(bkey('Bone028'))
    out['before'] = [round(t.translation.x, 3), round(t.translation.y, 3), round(t.translation.z, 3)]
    t.translation.z -= 2.0
    rh.set_global_transform(bkey('Bone028'), t, True)
    t2 = rh.get_global_transform(bkey('Bone028'))
    out['after'] = [round(t2.translation.x, 3), round(t2.translation.y, 3), round(t2.translation.z, 3)]
print('JSONSTART')
print(json.dumps(out, ensure_ascii=False))
print('JSONEND')
