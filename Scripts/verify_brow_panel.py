import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
rig = unreal.load_asset(ASSET)
h = rig.get_hierarchy()

controls = []
bones = []
curves = []
for k in h.get_all_keys():
    if k.type == unreal.RigElementType.CONTROL:
        st = h.get_control_settings(k)
        off = h.get_global_control_offset_transform(k)
        controls.append({
            'name': str(k.name),
            'display': str(st.display_name),
            'shape': str(st.shape_name),
            'z': round(off.translation.z, 2),
        })
    elif k.type == unreal.RigElementType.BONE:
        bones.append(str(k.name))
    elif k.type == unreal.RigElementType.CURVE:
        curves.append(str(k.name))

nodes = [n.get_name() for n in rig.get_default_model().get_nodes()]
out = {
    'status': str(rig.get_editor_property('status')),
    'controls': controls,
    'bones': bones,
    'curves': curves,
    'brow_nodes': [n for n in nodes if n.startswith('BROW_')],
    'dirty': rig.get_outermost().is_dirty() if hasattr(rig.get_outermost(), 'is_dirty') else 'n/a',
}
print(json.dumps(out, ensure_ascii=False, indent=1))
