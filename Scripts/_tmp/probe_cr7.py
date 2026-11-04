import json
import unreal

out = {}
a = unreal.load_asset('/Game/_MyTest/Mouth/Mesh/CR_Seven')
out['class'] = a.get_class().get_name() if a else None
if a:
    out['props'] = [p for p in dir(a) if 'rig' in p.lower() or 'control' in p.lower() or 'hierarchy' in p.lower()]
    for prop in ('ControlRigClass', 'RigClass', 'PreviewSkeletalMesh'):
        try:
            v = a.get_editor_property(prop)
            out[prop] = v.get_path_name() if hasattr(v, 'get_path_name') else str(v)
        except Exception as exc:
            out[prop] = 'ERR'
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
