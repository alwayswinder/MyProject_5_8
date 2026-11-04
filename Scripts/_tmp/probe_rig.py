import json
import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
rig = unreal.load_asset(ASSET)
h = rig.get_hierarchy()
out = {'status': str(rig.get_editor_property('status'))}
rows = []


def rot_info(t):
    q = t.rotation
    fx = unreal.MathLibrary.get_forward_vector(q)
    fy = unreal.MathLibrary.get_right_vector(q)
    fz = unreal.MathLibrary.get_up_vector(q)
    f = lambda v: [round(v.x, 3), round(v.y, 3), round(v.z, 3)]
    return {'quat': [round(q.x, 4), round(q.y, 4), round(q.z, 4), round(q.w, 4)],
            'X': f(fx), 'Y': f(fy), 'Z': f(fz)}


for k in h.get_all_keys():
    name = str(k.name)
    typ = str(k.type).replace('RigElementType.', '')
    row = {'type': typ, 'name': name}
    try:
        p = h.get_first_parent(k)
        row['parent'] = str(p.name) if p else ''
    except Exception as exc:
        row['parent'] = 'ERR %r' % exc
    try:
        row['global'] = h.get_global_transform(k)
        row['local'] = h.get_local_transform(k)
    except Exception as exc:
        row['xform_err'] = repr(exc)
    if typ == 'CONTROL':
        try:
            off = h.get_global_control_offset_transform(k)
            row['offset'] = off
            row['offset_rot'] = rot_info(off)
            st = h.get_control_settings(k)
            row['settings'] = {
                'type': str(st.control_type),
                'display': str(st.display_name),
                'shape': str(st.shape_name),
                'anim': str(st.animation_type),
                'primary': str(st.primary_axis) if hasattr(st, 'primary_axis') else '',
                'min': str(st.minimum_value),
                'max': str(st.maximum_value),
            }
        except Exception as exc:
            row['ctrl_err'] = repr(exc)
    if typ in ('BONE', 'NULL'):
        try:
            off = h.get_global_control_offset_transform(k)
            row['offset_rot'] = rot_info(off)
        except Exception:
            pass
    rows.append(row)

out['count'] = len(rows)
out['elements'] = rows
print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, default=str, indent=1))
print('JSONEND')
