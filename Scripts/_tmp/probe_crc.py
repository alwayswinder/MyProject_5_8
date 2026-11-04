import json
import unreal

out = {}
actor = None
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if a.get_actor_label() == 'CR_Seven_MorphSliders':
        actor = a
        break
out['found'] = bool(actor)
if actor:
    comps = []
    for c in actor.get_components_by_class(unreal.ControlRigComponent):
        comps.append({
            'name': c.get_name(),
            'class': c.get_class().get_name(),
            'methods': [m for m in dir(c) if 'control' in m.lower() or 'hierarchy' in m.lower()],
        })
    out['components'] = comps
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
