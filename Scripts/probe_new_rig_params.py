"""读新 rig 的 LID 圆弧参数（OQUAT axis / OANG 角度 / K0 缩放），确认是否沿用旧睁眼参数。"""
import json

import unreal

RIG = '/Game/_MyTest/Mouth/TMp/CR_Seven_MorphSliders1'

rig = unreal.load_asset(RIG)
ctrl = rig.get_controller(rig.get_default_model())
out = {}
for b in ('Bone027', 'Bone028', 'Bone029', 'Bone036', 'Bone037', 'Bone038'):
    rec = {}
    for pin in ('OQUAT_%s.Axis' % b, 'OANG_%s.B' % b, 'K0_%s.B' % b, 'SANG_%s.B' % b, 'SQUAT_%s.Axis' % b):
        try:
            rec[pin] = ctrl.get_pin_default_value('LID_' + pin)
        except Exception as exc:
            rec[pin] = 'ERR %r' % exc
    out[b] = rec

print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
