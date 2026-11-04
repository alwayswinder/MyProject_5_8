import json
import unreal

sk = unreal.load_asset('/Game/_MyTest/Mouth/Mesh/Seven_Skeleton')
bt = sk.get_editor_property('bone_tree')
rows = []
for x in bt:
    rows.append(str(x))
# 只挑关心的
wanted = ('head', 'meibu_L', 'meibu_R', 'Bone006', 'Bone007', 'Bone008', 'Bone009',
          'Bone010', 'Bone011', 'Bone012', 'Bone013', 'yankuang_L', 'yankuang_R', 'etou_L', 'etou_R')
sel = [r for r in rows if any(('\'' + w + '\'') in r or (' ' + w + ' ') in r or r.startswith(w + ' ')) for w in wanted]
print('JSONSTART')
print(json.dumps({'sample': rows[:3], 'sel': sel}, ensure_ascii=False, indent=1))
print('JSONEND')
