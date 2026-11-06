"""给新 rig 的 debugged instance 设置上眼皮闭合，便于截图（左眼 -10）。"""
import json

import unreal

RIG = '/Game/_MyTest/Mouth/TMp/CR_Seven_MorphSliders1'


def ckey(n):
    return unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=n)


rig = unreal.load_asset(RIG)
inst = rig.get_debugged_control_rig()
out = {'debugged': inst is not None}
if inst is not None:
    rh = inst.get_hierarchy()
    for c, v in (('CTRL_EYE_LID_L', -10.0), ('CTRL_EYE_LID_R', 0.0)):
        rh.set_control_value(ckey(c), rh.make_control_value_from_vector(unreal.Vector(0.0, 0.0, v)))
        out[c] = v
    inst.execute('Forwards Solve')
print('JSONSTART')
print(json.dumps(out, ensure_ascii=False))
print('JSONEND')
