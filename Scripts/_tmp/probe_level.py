import json
import unreal

out = {'actors': []}
world = unreal.EditorLevelLibrary.get_editor_world()
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    label = a.get_actor_label()
    cls = a.get_class().get_name()
    out['actors'].append([label, cls])
    if 'ControlRig' in cls:
        try:
            c = a.get_editor_property('control_rig')
            out.setdefault('rig_actors', []).append([label, str(c)])
        except Exception as exc:
            out.setdefault('rig_err', []).append([label, repr(exc)])
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
