import json
import unreal
seq = unreal.load_asset('/Game/CR_Seven_MorphSliders_Take9.CR_Seven_MorphSliders_Take9')
out = {'tracks': []}
def walk(t, depth):
    info = {'cls': t.get_class().get_name(), 'depth': depth, 'name': t.get_name()}
    try:
        rc = t.get_editor_property('ControlRigClass')
        info['rig_class'] = rc.get_path_name() if rc else None
    except Exception:
        pass
    try:
        info['display'] = str(t.get_editor_property('DisplayName'))
    except Exception:
        pass
    try:
        info['sections'] = [[s.get_class().get_name(), s.get_name()] for s in t.get_sections()]
    except Exception:
        pass
    out['tracks'].append(info)
    try:
        kids = t.get_child_tracks()
    except Exception:
        kids = []
    for c in kids or []:
        walk(c, depth + 1)
try:
    for t in seq.get_tracks():
        walk(t, 0)
except Exception as exc:
    out['err'] = repr(exc)
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
