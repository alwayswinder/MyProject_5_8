"""诊断用：打印 RigHierarchy 父子 API 与 LID 骨层级，并把 debugged rig 的上眼皮设到闭合，便于截图。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
BONES = ['Bone027', 'Bone028', 'Bone029', 'Bone036', 'Bone037', 'Bone038', 'Bone030', 'Bone053']


def bkey(name):
    return unreal.RigElementKey(type=unreal.RigElementType.BONE, name=name)


def ckey(name):
    return unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=name)


out = {}
rig = unreal.load_asset(ASSET)
h = rig.get_hierarchy()
out['hier_parent_api'] = [m for m in dir(h) if 'parent' in m.lower() or 'child' in m.lower()]
out['hier_all'] = [m for m in dir(h) if not m.startswith('_')]

for b in BONES:
    rec = {}
    try:
        rec['parent'] = str(h.get_first_parent(bkey(b)).name)
    except Exception as exc:
        rec['parent'] = 'ERR %r' % exc
    try:
        rec['children'] = [str(c.name) for c in h.get_children(bkey(b))]
    except Exception as exc:
        rec['children'] = 'ERR %r' % exc
    out[b] = rec

# 骨骼总数与是否有辅助骨名字含 lid/eye
keys = h.get_all_keys()
out['num_keys'] = len(keys)
out['eye_named_bones'] = sorted([str(k.name) for k in keys
                                 if k.type == unreal.RigElementType.BONE
                                 and any(t in str(k.name).lower() for t in ('lid', 'eye', 'eyelid', 'lash'))])

inst = rig.get_debugged_control_rig()
out['debugged'] = inst is not None
if inst is not None:
    rh = inst.get_hierarchy()
    for c, v in (('CTRL_EYE_LID_L', (0.0, 0.0, -10.0)), ('CTRL_EYE_LID_R', (0.0, 0.0, -10.0))):
        try:
            rh.set_control_value(ckey(c), rh.make_control_value_from_vector(unreal.Vector(*v)))
            out[c] = list(v)
        except Exception as exc:
            out[c] = 'ERR %r' % exc
    inst.execute('Forwards Solve')
    out['pose'] = {}
    for b in BONES:
        t = rh.get_global_transform(bkey(b))
        out['pose'][b] = [round(t.translation.x, 3), round(t.translation.y, 3), round(t.translation.z, 3)]

print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
