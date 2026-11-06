"""把 debugged rig 的上眼皮控制设为 -10（闭合到底），用于截图。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
VAL = -10.0


def ckey(n):
    return unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=n)


rig = unreal.load_asset(ASSET)
inst = rig.get_debugged_control_rig()
out = {'value': VAL, 'debugged': inst is not None}
if inst is not None:
    rh = inst.get_hierarchy()
    for c in ('CTRL_EYE_LID_L', 'CTRL_EYE_LID_R'):
        rh.set_control_value(ckey(c), rh.make_control_value_from_vector(unreal.Vector(0.0, 0.0, VAL)))
    inst.execute('Forwards Solve')
print('JSONSTART')
print(json.dumps(out, ensure_ascii=False))
print('JSONEND')
