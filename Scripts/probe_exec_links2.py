"""读 RigVM 连线（用 get_source_pin/get_target_pin），检查 Begin 执行流与 BROW 链。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'

rig = unreal.load_asset(ASSET)
model = rig.get_default_model()
links = list(model.get_links())

pairs = []
for link in links:
    try:
        s = str(link.get_pin_path_representation(link.get_source_pin()))
    except Exception:
        try:
            s = str(link.get_source_pin())
        except Exception:
            s = None
    try:
        d = str(link.get_pin_path_representation(link.get_target_pin()))
    except Exception:
        try:
            d = str(link.get_target_pin())
        except Exception:
            d = None
    pairs.append([s, d])

out = {'count': len(pairs), 'sample': pairs[:6]}

exec_pairs = [p for p in pairs if p[0] and (p[0].endswith('.Execute') or '.Execute.' in p[0])]
out['exec_sample'] = exec_pairs[:40]
out['exec_count'] = len(exec_pairs)

brow = [p for p in pairs if (p[0] and 'BROW' in p[0]) or (p[1] and 'BROW' in p[1])]
out['brow_count'] = len(brow)
out['brow_sample'] = brow[:40]

begin = [p for p in pairs if (p[0] and p[0].startswith('Begin')) or (p[1] and p[1].startswith('Begin'))]
out['begin_links'] = begin

print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
