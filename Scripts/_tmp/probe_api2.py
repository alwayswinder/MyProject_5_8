import json
import unreal

rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
h = rig.get_hierarchy()
k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_POINT')
st = h.get_control_settings(k)
out = {}
out['h_methods'] = [m for m in dir(h) if 'control' in m.lower()]
out['settings_attrs'] = [a for a in dir(st) if not a.startswith('_')]
out['rig_methods'] = [m for m in dir(rig) if 'hierarchy' in m.lower() or 'recompile' in m.lower()]
out['ctrl_type_enum'] = [e for e in dir(unreal.RigControlType) if not e.startswith('_')]
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
