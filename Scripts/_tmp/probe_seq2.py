import json
import unreal

out = {'tracks': []}
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if a.get_class().get_name() != 'LevelSequenceActor':
        continue
    seq = a.get_sequence()
    out['actor'] = a.get_actor_label()
    out['sequence'] = seq.get_path_name() if seq else None
    if not seq:
        continue
    try:
        ms = seq.get_editor_property('MovieScene')
    except Exception:
        ms = None
    if ms is None:
        out['err'] = 'no movie scene'
        continue
    def walk(t, depth=0):
        info = {'cls': t.get_class().get_name(), 'depth': depth}
        try:
            crc = t.get_editor_property('ControlRigClass')
            info['rig_class'] = str(crc.get_path_name()) if crc else None
        except Exception:
            pass
        try:
            info['display'] = str(t.get_editor_property('DisplayName'))
        except Exception:
            pass
        out['tracks'].append(info)
        try:
            kids = t.get_editor_property('Tracks')
        except Exception:
            kids = []
        for c in kids or []:
            walk(c, depth+1)
    for t in ms.get_tracks():
        walk(t)
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
