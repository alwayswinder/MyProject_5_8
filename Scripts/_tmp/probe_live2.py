import json
import unreal

out = {}
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if a.get_actor_label() != 'CR_Seven_MorphSliders':
        continue
    for c in a.get_components_by_class(unreal.SkeletalMeshComponent):
        res = {}
        for b in ('head', 'meibu_L', 'meibu_R'):
            t = c.get_bone_transform(b, unreal.RelativeTransformSpace.RTS_WORLD)
            res[b] = [round(t.translation.x, 2), round(t.translation.y, 2), round(t.translation.z, 2)]
        out['live'] = res
        out['rel_L'] = [round(res['meibu_L'][i] - res['head'][i], 3) for i in range(3)]
        out['rel_R'] = [round(res['meibu_R'][i] - res['head'][i], 3) for i in range(3)]
        out['rest_rel_L'] = [4.458, 9.813, 10.04]
        out['rest_rel_R'] = [-4.457, 9.813, 10.04]
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
