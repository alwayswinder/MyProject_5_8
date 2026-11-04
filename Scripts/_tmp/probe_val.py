import json
import unreal

rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_POINT')
out = {}

def show(tag, h):
    t = h.get_global_transform(k).translation
    out[tag] = [round(t.x, 3), round(t.y, 3), round(t.z, 3)]

i = rig.create_control_rig(); i.request_init(); rh = i.get_hierarchy()
show('base', rh)

try:
    v = rh.make_control_value_from_vector(unreal.Vector(10, 0, 0))
    rh.set_control_value(k, v)
    show('vector_value', rh)
    out['read_back'] = str(rh.get_control_value(k))
except Exception as exc:
    out['vec_err'] = repr(exc)

try:
    v2 = rh.make_control_value_from_transform(unreal.Transform(location=unreal.Vector(0, 10, 0)))
    rh.set_control_value(k, v2)
    show('transform_value', rh)
except Exception as exc:
    out['tr_err'] = repr(exc)

# 直接设 transform
try:
    rh.set_control_transform(k, unreal.Transform(location=unreal.Vector(0, 20, 0))) if hasattr(rh, 'set_control_transform') else None
    show('set_control_transform', rh)
except Exception as exc:
    out['sct_err'] = repr(exc)

out['has_set_transform'] = hasattr(rh, 'set_control_transform')
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
