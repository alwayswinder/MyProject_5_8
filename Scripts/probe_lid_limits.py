"""验证上眼皮控制限值：X/Y 必须为 0，Z 只能在 [-10, 0]。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
CONTROLS = ('CTRL_EYE_LID_L', 'CTRL_EYE_LID_R')


def ckey(n):
    return unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=n)


def bkey(n):
    return unreal.RigElementKey(type=unreal.RigElementType.BONE, name=n)


rig = unreal.load_asset(ASSET)
inst = rig.create_control_rig()
inst.request_init()
rh = inst.get_hierarchy()

out = {'cases': {}, 'bones': {}}
for name in CONTROLS:
    cases = {}
    for raw in ((10.0, 10.0, 10.0), (5.0, -5.0, -5.0), (0.0, 0.0, -100.0), (0.0, 0.0, -10.0), (0.0, 0.0, 0.0)):
        rh.set_control_value(ckey(name), rh.make_control_value_from_vector(unreal.Vector(*raw)))
        v = rh.get_vector_from_control_value(rh.get_control_value(ckey(name)))
        cases['%s' % (list(raw),)] = [round(v.x, 3), round(v.y, 3), round(v.z, 3)]
    out['cases'][name] = cases

# 骨骼检查：Z=-10 应复现 Max 端点
rh.set_control_value(ckey('CTRL_EYE_LID_L'), rh.make_control_value_from_vector(unreal.Vector(0.0, 0.0, -10.0)))
rh.set_control_value(ckey('CTRL_EYE_LID_R'), rh.make_control_value_from_vector(unreal.Vector(0.0, 0.0, -10.0)))
inst.execute('Forwards Solve')
for b in ('Bone028', 'Bone037'):
    t = rh.get_global_transform(bkey(b)).translation
    out['bones'][b] = [round(t.x, 3), round(t.y, 3), round(t.z, 3)]

print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
