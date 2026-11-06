"""对比新旧 rig 的骨骼集合（新网格少 16 根，看少的是哪些）。"""
import json

import unreal

OLD = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
NEW = '/Game/_MyTest/Mouth/TMp/CR_Seven_MorphSliders1'


def bones(path):
    rig = unreal.load_asset(path)
    h = rig.get_hierarchy()
    return sorted([str(k.name) for k in h.get_all_keys() if k.type == unreal.RigElementType.BONE])


old = set(bones(OLD))
new = set(bones(NEW))
out = {
    'old_count': len(old),
    'new_count': len(new),
    'missing_in_new': sorted(old - new),
    'added_in_new': sorted(new - old),
}
print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
