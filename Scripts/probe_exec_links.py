"""读 RigVM 图的连线，检查执行流是否断开（BROW 链是否还挂在 Begin 上）。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'

rig = unreal.load_asset(ASSET)
model = rig.get_default_model()
out = {}

links = list(model.get_links())
out['link_count'] = len(links)
sample = links[0] if links else None
attrs = [a for a in dir(sample) if not a.startswith('_')] if sample else []
out['link_attrs'] = attrs

def pin_of(link):
    for a in ('source_pin_path', 'source_pin', 'source', 'output_pin'):
        try:
            v = link.get_editor_property(a)
            return str(v)
        except Exception:
            continue
    return None

def dst_of(link):
    for a in ('target_pin_path', 'target_pin', 'target', 'input_pin'):
        try:
            v = link.get_editor_property(a)
            return str(v)
        except Exception:
            continue
    return None

pairs = []
for link in links:
    s = pin_of(link)
    d = dst_of(link)
    if s and d:
        pairs.append([s, d])

out['pair_count'] = len(pairs)
out['sample_pairs'] = pairs[:5]

# Begin 的执行输出
exec_links = [[s, d] for s, d in pairs if 'Begin' in s or 'Execute' in s or 'Execute' in d]
out['exec_links'] = exec_links[:60]

# 与 BROW 相关的输入连线
brow_links = [[s, d] for s, d in pairs if 'BROW' in s or 'BROW' in d]
out['brow_links'] = brow_links[:40]

print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
