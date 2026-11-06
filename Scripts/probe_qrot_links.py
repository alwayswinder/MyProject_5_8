"""读 codex 版 LID_QROT 的输入连接，确认旋转用的是公转四元数（OQUAT）还是自转（SQUAT）。"""
import json

import unreal

RIG = '/Game/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation'

rig = unreal.load_asset(RIG)
model = rig.get_default_model()

targets = {}
for link in model.get_links():
    try:
        tn = str(link.get_target_node().get_name())
        tp = str(link.get_target_pin().get_name())
        sn = str(link.get_source_node().get_name())
        sp = str(link.get_source_pin().get_name())
    except Exception:
        continue
    if tn.startswith('LID_QROT') or tn.startswith('LID_SQ') or tn.startswith('LID_OQ'):
        targets.setdefault(tn, []).append({'pin': tp, 'from': '%s.%s' % (sn, sp)})

print('JSONSTART')
print(json.dumps(targets, ensure_ascii=False, indent=1))
print('JSONEND')
