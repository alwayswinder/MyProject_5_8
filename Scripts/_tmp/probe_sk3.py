import json
import unreal

sk = unreal.load_asset('/Game/_MyTest/Mouth/Mesh/Seven_Skeleton')
bt = sk.get_editor_property('bone_tree')
rows = [str(x) for x in bt]
wanted = ('head', 'meibu_L', 'meibu_R', 'Bone006', 'Bone007', 'Bone008', 'Bone009',
          'Bone010', 'Bone011', 'Bone012', 'Bone013', 'yankuang_L', 'yankuang_R', 'etou_L', 'etou_R')
sel = []
for r in rows:
    for wname in wanted:
        if r.startswith(wname) or ("'" + wname + "'") in r:
            sel.append(r)
            break
print('JSONSTART')
print(json.dumps({'first': rows[:2], 'sel': sel}, ensure_ascii=False, indent=1)[:4000])
print('JSONEND')
