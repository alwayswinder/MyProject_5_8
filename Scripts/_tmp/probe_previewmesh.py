import json
import unreal

rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
h = rig.get_hierarchy()
k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_POINT')
out = {}
pm = None
try:
    pm = rig.get_preview_mesh()
    out['preview_mesh'] = str(pm)
except Exception as exc:
    out['preview_mesh_err'] = repr(exc)

def bones():
    if not pm:
        return None
    res = {}
    for b in ('meibu_L', 'meibu_R'):
        try:
            t = pm.get_bone_transform(b, unreal.RelativeTransformSpace.RTS_WORLD)
            res[b] = [round(t.translation.x, 2), round(t.translation.y, 2), round(t.translation.z, 2)]
        except Exception as exc:
            res[b] = 'ERR'
    return res

h.set_control_value(k, unreal.RigHierarchy.make_control_value_from_vector2d(unreal.Vector2D(0.0, 0.0)))
out['at0'] = bones()
h.set_control_value(k, unreal.RigHierarchy.make_control_value_from_vector2d(unreal.Vector2D(0.0, 10.0)))
out['atY10'] = bones()
h.set_control_value(k, unreal.RigHierarchy.make_control_value_from_vector2d(unreal.Vector2D(10.0, 0.0)))
out['atX10'] = bones()
h.set_control_value(k, unreal.RigHierarchy.make_control_value_from_vector2d(unreal.Vector2D(0.0, 0.0)))
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
