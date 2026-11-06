"""实验：把 Bone028 的旋转增益临时放大 100 倍，判断上眼皮网格是否真的跟随该骨骼（验证权重）。
执行后请运行 restore_lid_gain.py 复原（不保存资产）。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def ckey(n):
    return unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=n)


def bkey(n):
    return unreal.RigElementKey(type=unreal.RigElementType.BONE, name=n)


rig = unreal.load_asset(ASSET)
ctrl = rig.get_controller(rig.get_default_model())
out = {}
EXPECT = ['Bone027', 'Bone028', 'Bone029', 'Bone036', 'Bone037', 'Bone038']
for b in EXPECT:
    pin = 'LID_ANG_%s.B' % b
    try:
        old = ctrl.get_pin_default_value(pin)
        out[b + '_old'] = old
    except Exception as exc:
        out[b + '_old'] = 'ERR %r' % exc

for b in ('Bone028', 'Bone037'):
    ctrl.set_pin_default_value('LID_ANG_%s.B' % b, '20.796', False, False)
rig.recompile_vm()

inst = rig.get_debugged_control_rig()
rh = inst.get_hierarchy()
for c in ('CTRL_EYE_LID_L', 'CTRL_EYE_LID_R'):
    rh.set_control_value(ckey(c), rh.make_control_value_from_vector(unreal.Vector(0.0, 0.0, -1.0)))
inst.execute('Forwards Solve')
for b in ('Bone028', 'Bone037'):
    q = rh.get_global_transform(bkey(b)).rotation
    out[b] = [round(q.x, 3), round(q.y, 3), round(q.z, 3), round(q.w, 3)]
out['control'] = -1.0
print('JSONSTART')
print(json.dumps(out, ensure_ascii=False))
print('JSONEND')
