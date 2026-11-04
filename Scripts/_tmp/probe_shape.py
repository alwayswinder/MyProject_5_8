import json
import unreal
rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
h = rig.get_hierarchy()
k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_POINT')
out = {}
for tag, t in (('global_shape', h.get_global_control_shape_transform(k)),
               ('local_shape', h.get_local_control_shape_transform(k))):
    out[tag] = {'loc': [round(t.translation.x,3), round(t.translation.y,3), round(t.translation.z,3)],
                'rot': [round(t.rotation.x,3), round(t.rotation.y,3), round(t.rotation.z,3), round(t.rotation.w,3)],
                'scale': [round(t.scale3d.x,5), round(t.scale3d.y,5), round(t.scale3d.z,5)]}
print('JSONSTART'); print(json.dumps(out, indent=1)); print('JSONEND')
