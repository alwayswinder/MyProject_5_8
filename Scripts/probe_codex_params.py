"""读 codex 版 SQUAT 的轴/增益，和 OQUAT 的轴/角度对比。"""
import json

import unreal

RIG = '/Game/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation'

rig = unreal.load_asset(RIG)
ctrl = rig.get_controller(rig.get_default_model())
out = {}
for b in ('Bone027', 'Bone028', 'Bone029', 'Bone036', 'Bone037', 'Bone038'):
    rec = {}
    for pin in ('OQUAT_%s.Axis' % b, 'OANG_%s.B' % b, 'SQUAT_%s.Axis' % b, 'SANG_%s.B' % b):
        try:
            rec[pin] = ctrl.get_pin_default_value('LID_' + pin)
        except Exception as exc:
            rec[pin] = 'ERR %r' % exc
    out[b] = rec

print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
