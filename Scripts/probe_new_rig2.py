"""新 rig：读眼皮骨初始位置、LID 节点清单（查重复）、以及 0 / -10 两个状态。"""
import json

import unreal

RIG = '/Game/_MyTest/Mouth/TMp/CR_Seven_MorphSliders1'
BONES = ['Bone027', 'Bone028', 'Bone029', 'Bone036', 'Bone037', 'Bone038']


def ckey(n):
    return unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=n)


def bkey(n):
    return unreal.RigElementKey(type=unreal.RigElementType.BONE, name=n)


rig = unreal.load_asset(RIG)
model = rig.get_default_model()
names = [n.get_name() for n in model.get_nodes()]
out = {'lid_nodes': sorted([n for n in names if n.startswith('LID_')])}

inst = rig.create_control_rig()
inst.request_init()
rh = inst.get_hierarchy()


def pos(b):
    t = rh.get_global_transform(bkey(b)).translation
    return [round(t.x, 4), round(t.y, 4), round(t.z, 4)]


def rot(b):
    q = rh.get_global_transform(bkey(b)).rotation
    return [round(q.x, 4), round(q.y, 4), round(q.z, 4), round(q.w, 4)]


for c in ('CTRL_EYE_LID_L', 'CTRL_EYE_LID_R'):
    rh.set_control_value(ckey(c), rh.make_control_value_from_vector(unreal.Vector(0.0, 0.0, 0.0)))
inst.execute('Forwards Solve')
out['rest'] = {b: {'t': pos(b), 'q': rot(b)} for b in BONES}

for c in ('CTRL_EYE_LID_L', 'CTRL_EYE_LID_R'):
    rh.set_control_value(ckey(c), rh.make_control_value_from_vector(unreal.Vector(0.0, 0.0, -10.0)))
inst.execute('Forwards Solve')
out['closed'] = {b: {'t': pos(b)} for b in BONES}

print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
