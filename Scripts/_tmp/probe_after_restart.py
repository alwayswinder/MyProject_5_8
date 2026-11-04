import json
import unreal

out = {}
rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
h = rig.get_hierarchy()
k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_POINT')
if h.contains(k):
    st = h.get_control_settings(k)
    off = h.get_global_control_offset_transform(k)
    out['control'] = {
        'type': str(st.control_type),
        'display': str(st.display_name),
        'offset_rot': [round(off.rotation.x,3), round(off.rotation.z,3), round(off.rotation.w,3)],
        'offset_pos': [round(off.translation.x,2), round(off.translation.y,2), round(off.translation.z,2)],
        'scale': round(off.scale3d.x,2),
    }
out['nodes'] = [n.get_name() for n in rig.get_default_model().get_nodes() if n.get_name().startswith('BRW')]

for a in unreal.EditorLevelLibrary.get_all_level_actors():
    lab = a.get_actor_label()
    if lab != 'CR_Seven_MorphSliders':
        continue
    comps = a.get_components_by_class(unreal.SkeletalMeshComponent)
    out['mesh_components'] = [c.get_name() for c in comps]
    for c in comps:
        res = {}
        for b in ('head', 'meibu_L', 'meibu_R'):
            t = c.get_bone_transform(b, unreal.RelativeTransformSpace.RTS_WORLD)
            res[b] = [round(t.translation.x, 2), round(t.translation.y, 2), round(t.translation.z, 2)]
        out['live_' + c.get_name()] = res
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
