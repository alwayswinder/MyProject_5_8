import json
import unreal

rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
h = rig.get_hierarchy()
out = {'elements': []}
for k in h.get_all_keys():
    name = str(k.name)
    t = h.get_global_transform(k).translation
    out['elements'].append({'name': name, 'type': str(k.type).split(':')[0].strip('<'),
                            'global': [round(t.x,2), round(t.y,2), round(t.z,2)]})
k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_POINT')
try:
    v = h.get_control_value(k)
    out['control_value_raw'] = repr(v)
    out['as_vector'] = str(h.get_transform_from_control_value(v))
except Exception as exc:
    out['value_err'] = repr(exc)
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
