import json
import unreal

out = {}
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if a.get_actor_label() != 'CR_Seven_MorphSliders':
        continue
    comps = [c for c in a.get_components_by_class(unreal.SkeletalMeshComponent)]
    out['components'] = [c.get_name() for c in comps]
    for c in comps:
        bones = ['head', 'meibu_L', 'meibu_R', 'yankuang_L']
        res = {}
        for b in bones:
            try:
                t = c.get_bone_transform(b, unreal.RelativeTransformSpace.RTS_WORLD)
                res[b] = [round(t.translation.x, 2), round(t.translation.y, 2), round(t.translation.z, 2)]
            except Exception as exc:
                res[b] = 'ERR ' + repr(exc)[:60]
        out[c.get_name()] = res
        try:
            out['anim_mode'] = str(c.get_editor_property('AnimationMode'))
        except Exception:
            pass
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
