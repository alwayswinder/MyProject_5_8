"""把 debugged rig 的上眼皮控制设为指定值（命令行参数），用于截图对比。"""
import json
import sys

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def ckey(name):
    return unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=name)


def bkey(name):
    return unreal.RigElementKey(type=unreal.RigElementType.BONE, name=name)


val = float(sys.argv[1]) if len(sys.argv) > 1 else 0.0
rig = unreal.load_asset(ASSET)
inst = rig.get_debugged_control_rig()
out = {'value': val, 'debugged': inst is not None}
if inst is not None:
    rh = inst.get_hierarchy()
    for c in ('CTRL_EYE_LID_L', 'CTRL_EYE_LID_R'):
        try:
            rh.set_control_value(ckey(c), rh.make_control_value_from_vector(unreal.Vector(0.0, 0.0, val)))
        except Exception as exc:
            out[c] = 'ERR %r' % exc
    inst.execute('Forwards Solve')
    out['Bone028'] = [round(v, 3) for v in (
        rh.get_global_transform(bkey('Bone028')).translation.x,
        rh.get_global_transform(bkey('Bone028')).translation.y,
        rh.get_global_transform(bkey('Bone028')).translation.z)]
print('JSONSTART')
print(json.dumps(out, ensure_ascii=False))
print('JSONEND')
