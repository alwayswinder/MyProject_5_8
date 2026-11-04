import json
import unreal

mesh = unreal.load_asset('/Game/_MyTest/Mouth/Mesh/Seven')
ref = unreal.new_object(unreal.RigHierarchy)
ref.get_controller(True).import_bones_from_skeletal_mesh(mesh, 'None', True, True, False, False, False)
out = {}
for k in ref.get_all_keys():
    name = str(k.name)
    if name in ('head', 'meibu_L', 'meibu_R', 'Bone006', 'Bone007', 'Bone008', 'Bone009',
                'Bone010', 'Bone011', 'Bone012', 'Bone013', 'yankuang_L', 'yankuang_R', 'etou_L', 'etou_R'):
        p = ref.get_first_parent(k)
        out[name] = str(p.name) if p else None
# 有没有以 meibu 为父的骨骼
children = {}
for k in ref.get_all_keys():
    p = ref.get_first_parent(k)
    if p and str(p.name) in ('meibu_L', 'meibu_R'):
        children.setdefault(str(p.name), []).append(str(k.name))
out['children_of_meibu'] = children
out['total'] = len(ref.get_all_keys())
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
