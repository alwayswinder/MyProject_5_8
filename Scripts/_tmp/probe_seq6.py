import json
import unreal

out = {}
lib = unreal.LevelSequenceEditorBlueprintLibrary
cur = lib.get_current_level_sequence()
out['current'] = cur.get_path_name() if cur else None

seqs = []
if cur:
    seqs.append(('current', cur))
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if a.get_class().get_name() == 'LevelSequenceActor':
        s = a.get_sequence()
        if s:
            seqs.append((a.get_actor_label(), s))

out['seqs'] = []
for tag, s in seqs:
    info = {'tag': tag, 'path': s.get_path_name(), 'tracks': []}
    def walk(t, d):
        row = {'cls': t.get_class().get_name(), 'd': d}
        for prop in ('ControlRigClass', 'DisplayName'):
            try:
                v = t.get_editor_property(prop)
                row[prop] = v.get_path_name() if hasattr(v, 'get_path_name') else str(v)
            except Exception:
                pass
        info['tracks'].append(row)
        try:
            for c in (t.get_child_tracks() or []):
                walk(c, d+1)
        except Exception:
            pass
    try:
        for t in s.get_tracks():
            walk(t, 0)
    except Exception as exc:
        info['err'] = repr(exc)
    out['seqs'].append(info)
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)[:6000]); print('JSONEND')
