import json
import unreal

out = {}
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if a.get_class().get_name() != 'LevelSequenceActor':
        continue
    seq = a.get_sequence()
    out['actor'] = a.get_actor_label()
    out['sequence'] = seq.get_path_name() if seq else None
    if not seq:
        continue
    ms = seq.get_editor_property('movie_scene')
    tracks = []
    def walk(t, depth=0):
        info = {'cls': t.get_class().get_name(), 'name': t.get_name() if hasattr(t, 'get_name') else '', 'depth': depth}
        try:
            crc = t.get_editor_property('control_rig_class')
            info['rig_class'] = str(crc.get_path_name()) if crc else None
        except Exception:
            pass
        tracks.append(info)
        try:
            for s in t.get_sections():
                pass
        except Exception:
            pass
        try:
            for c in t.get_child_tracks() if hasattr(t, 'get_child_tracks') else []:
                walk(c, depth+1)
        except Exception:
            pass
    try:
        for t in ms.get_tracks():
            walk(t)
    except Exception as exc:
        out['track_err'] = repr(exc)
    out['tracks'] = tracks
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
