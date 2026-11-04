import json
import unreal

rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
h = rig.get_hierarchy()
k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_POINT')
out = {}
st = h.get_control_settings(k)
off = h.get_global_control_offset_transform(k)
out['control'] = {'type': str(st.control_type), 'display': str(st.display_name),
                  'offset_pos': [round(off.translation.x,2), round(off.translation.y,2), round(off.translation.z,2)],
                  'offset_rot': [round(off.rotation.x,3), round(off.rotation.y,3), round(off.rotation.z,3), round(off.rotation.w,3)],
                  'scale': round(off.scale3d.x,2)}

def pin_name(p):
    s = str(p)
    return s.split('.')[-1].strip("'> ")

nodes = []
for n in rig.get_default_model().get_nodes():
    info = {'name': n.get_name(), 'cls': n.get_class().get_name(), 'pins': []}
    for p in n.get_pins():
        try:
            dv = p.get_default_value()
        except Exception:
            dv = None
        links = []
        try:
            for l in p.get_links():
                links.append(str(l).split('RigVMModel.')[-1].strip("'> "))
        except Exception:
            pass
        if dv not in (None, '', '()') or links:
            info['pins'].append({'pin': pin_name(p), 'default': dv, 'links': links})
    nodes.append(info)
out['nodes'] = nodes
print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
