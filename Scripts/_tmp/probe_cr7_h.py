import json
import unreal

a = unreal.load_asset('/Game/_MyTest/Mouth/Mesh/CR_Seven')
h = a.get_editor_property('hierarchy')
out = {'elements': []}
for k in h.get_all_keys():
    row = {'name': str(k.name), 'type': str(k.type).split(':')[0].strip('<')}
    try:
        t = h.get_global_transform(k)
        row['global'] = [round(t.translation.x,2), round(t.translation.y,2), round(t.translation.z,2)]
    except Exception:
        pass
    out['elements'].append(row)
    if k.type == unreal.RigElementType.CONTROL:
        try:
            st = h.get_control_settings(k)
            row['control_type'] = str(st.control_type)
        except Exception as exc:
            row['err'] = repr(exc)
        try:
            off = h.get_global_control_offset_transform(k)
            row['offset_rot'] = [round(off.rotation.x,3), round(off.rotation.y,3), round(off.rotation.z,3), round(off.rotation.w,3)]
        except Exception:
            pass
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
