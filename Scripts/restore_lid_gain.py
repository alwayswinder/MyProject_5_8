"""复原 probe_lid_weight_rot.py 对 LID_ANG 增益的临时修改，并把上眼皮控制归零。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
ORIG = {'Bone028': '0.207960', 'Bone037': '0.207960'}


def ckey(n):
    return unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=n)


rig = unreal.load_asset(ASSET)
ctrl = rig.get_controller(rig.get_default_model())
out = {}
for b, v in ORIG.items():
    pin = 'LID_ANG_%s.B' % b
    ctrl.set_pin_default_value(pin, v, False, False)
    out[b] = ctrl.get_pin_default_value(pin)
rig.recompile_vm()
out['status'] = str(rig.get_editor_property('status'))

inst = rig.get_debugged_control_rig()
if inst is not None:
    rh = inst.get_hierarchy()
    for c in ('CTRL_EYE_LID_L', 'CTRL_EYE_LID_R'):
        rh.set_control_value(ckey(c), rh.make_control_value_from_vector(unreal.Vector(0.0, 0.0, 0.0)))
    inst.execute('Forwards Solve')
    out['lid'] = 0.0
print('JSONSTART')
print(json.dumps(out, ensure_ascii=False))
print('JSONEND')
