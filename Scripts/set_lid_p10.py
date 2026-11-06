"""固定把 debugged rig 的上眼皮控制设为 +10（向上拖），看是否反向翻转穿模。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
VAL = 10.0


def ckey(n):
    return unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=n)


def bkey(n):
    return unreal.RigElementKey(type=unreal.RigElementType.BONE, name=n)


rig = unreal.load_asset(ASSET)
inst = rig.get_debugged_control_rig()
out = {'target': VAL, 'debugged': inst is not None}
if inst is not None:
    rh = inst.get_hierarchy()
    for c in ('CTRL_EYE_LID_L', 'CTRL_EYE_LID_R'):
        rh.set_control_value(ckey(c), rh.make_control_value_from_vector(unreal.Vector(0.0, 0.0, VAL)))
    inst.execute('Forwards Solve')
    for c in ('CTRL_EYE_LID_L', 'CTRL_EYE_LID_R'):
        try:
            v = rh.get_vector_from_control_value(rh.get_control_value(ckey(c)))
            out[c] = [round(v.x, 3), round(v.y, 3), round(v.z, 3)]
        except Exception as exc:
            out[c] = 'ERR %r' % exc
    t = rh.get_global_transform(bkey('Bone028'))
    out['Bone028'] = [round(t.translation.x, 3), round(t.translation.y, 3), round(t.translation.z, 3)]
print('JSONSTART')
print(json.dumps(out, ensure_ascii=False))
print('JSONEND')
