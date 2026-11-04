import json
import unreal

out = {}
actor = None
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if a.get_actor_label() == 'CR_Seven_MorphSliders':
        actor = a
        break
if actor:
    out['actor_class'] = actor.get_class().get_name()
    out['props'] = [p for p in dir(actor) if 'rig' in p.lower() or 'control' in p.lower()]
    try:
        comp = actor.get_editor_property('control_rig')
        out['control_rig'] = str(comp)
        out['rig_class'] = comp.get_class().get_name() if comp else None
        if comp:
            out['rig_methods'] = [m for m in dir(comp) if 'control' in m.lower() or 'hierarchy' in m.lower() or 'exec' in m.lower()]
    except Exception as exc:
        out['err'] = repr(exc)
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
