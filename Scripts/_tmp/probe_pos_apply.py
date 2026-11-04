import json
import unreal

rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_POINT')
out = {}
i = rig.create_control_rig()
i.request_init()
rh = i.get_hierarchy()
out['before_global'] = str(rh.get_global_transform(k))
rh.set_control_value(k, rh.make_control_value_from_transform(unreal.Transform(location=unreal.Vector(10, 0, 0))))
out['after_value_global'] = str(rh.get_global_transform(k))
out['offset'] = str(rh.get_global_control_offset_transform(k))
i.execute('Forwards Solve')
for n in ('meibu_L', 'meibu_R'):
    out[n] = str(rh.get_global_transform(unreal.RigElementKey(type=unreal.RigElementType.BONE, name=n)))
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
