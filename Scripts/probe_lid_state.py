"""诊断上眼皮穿模：读取 LID 链结构、骨骼层级、网格 morph / 权重可用性。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
MESH = '/Game/_MyTest/Mouth/Mesh/Seven'
BONES = ['Bone027', 'Bone028', 'Bone029', 'Bone036', 'Bone037', 'Bone038']

out = {}
rig = unreal.load_asset(ASSET)
h = rig.get_hierarchy()
model = rig.get_default_model()

names = [n.get_name() for n in model.get_nodes()]
out['node_count'] = len(names)
out['lid_nodes'] = sorted([n for n in names if n.startswith('LID_')])

links = []
try:
    for link in model.get_links():
        src = str(link.get_editor_property('source_pin_path'))
        dst = str(link.get_editor_property('target_pin_path'))
        if 'LID_QROT' in dst or 'LID_QROT' in src or 'LID_ADD' in dst:
            links.append([src, dst])
except Exception as exc:
    out['links_err'] = repr(exc)
out['lid_links'] = links

out['bones'] = {}
for b in BONES:
    k = unreal.RigElementKey(type=unreal.RigElementType.BONE, name=b)
    try:
        parent = h.get_parent(k)
        parent_name = str(parent.name) if parent else None
    except Exception as exc:
        parent_name = 'ERR %r' % exc
    t = h.get_global_transform(k)
    out['bones'][b] = {
        'parent': parent_name,
        'children': [str(c.name) for c in h.get_children(k)],
        't': [round(t.translation.x, 4), round(t.translation.y, 4), round(t.translation.z, 4)],
        'r': [round(t.rotation.x, 4), round(t.rotation.y, 4), round(t.rotation.z, 4), round(t.rotation.w, 4)],
        's': [round(t.scale3d.x, 4), round(t.scale3d.y, 4), round(t.scale3d.z, 4)],
    }

mesh = unreal.load_asset(MESH)
out['mesh'] = {}
out['mesh']['path'] = mesh.get_path_name()
for attr in ('morph_targets', 'morph_target_names', 'skeleton'):
    try:
        v = mesh.get_editor_property(attr)
        out['mesh'][attr] = str(v)[:500]
    except Exception as exc:
        out['mesh'][attr] = 'ERR %r' % exc
try:
    out['mesh']['morph_names'] = list(mesh.get_all_morph_target_names())
except Exception as exc:
    out['mesh']['morph_names'] = 'ERR %r' % exc
try:
    out['mesh']['num_verts'] = len(mesh.get_editor_property('lod_info')[0].get_editor_property('screen_size'))
except Exception as exc:
    out['mesh']['num_verts'] = 'ERR %r' % exc

# skin weight API 探测
out['skin_api'] = {}
for attr in ('get_vertex_weight', 'get_skin_weights', 'get_vertex_weights', 'get_skin_weight'):
    out['skin_api'][attr] = hasattr(mesh, attr)

# 是否启用了 DQS / 骨骼影响数
for attr in ('default_skin_weight', 'skin_weight_profiles', 'use_high_precision_skin_weights', 'b_use_dqs'):
    try:
        out['mesh'][attr] = str(mesh.get_editor_property(attr))[:300]
    except Exception:
        pass

print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
