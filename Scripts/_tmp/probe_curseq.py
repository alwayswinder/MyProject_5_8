import json
import unreal

out = {}
lib = unreal.LevelSequenceEditorBlueprintLibrary
seq = lib.get_current_level_sequence()
out['current_seq'] = seq.get_path_name() if seq else None
if seq:
    tracks = []
    def walk(t, depth):
        info = {'cls': t.get_class().get_name(), 'depth': depth, 'name': t.get_name()}
        for prop in ('ControlRigClass', 'DisplayName'):
            try:
                v = t.get_editor_property(prop)
                info[prop] = v.get_path_name() if hasattr(v, 'get_path_name') else str(v)
            except Exception:
                pass
        tracks.append(info)
        try:
            kids = t.get_child_tracks()
        except Exception:
            kids = []
        for c in kids or []:
            walk(c, depth + 1)
    for t in seq.get_tracks():
        walk(t, 0)
    out['tracks'] = tracks
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
