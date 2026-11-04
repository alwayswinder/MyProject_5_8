import json
import unreal

mesh = unreal.load_asset('/Game/_MyTest/Mouth/Mesh/Seven')
sk = mesh.get_editor_property('skeleton')
out = {}
names = ['head', 'meibu_L', 'meibu_R', 'Bone006', 'Bone007', 'Bone008', 'Bone009',
         'Bone010', 'Bone011', 'Bone012', 'Bone013', 'yankuang_L', 'yankuang_R']
parents = {}
for n in names:
    try:
        parents[n] = str(sk.get_parent_bone_name(n))
    except Exception as exc:
        parents[n] = 'ERR ' + repr(exc)[:60]
out['parents'] = parents
# 眉毛相关骨骼有哪些
allb = []
try:
    for i in range(sk.get_num_bones() if hasattr(sk, 'get_num_bones') else 0):
        pass
except Exception:
    pass
out['ref_pose'] = str(sk.get_reference_pose())
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
