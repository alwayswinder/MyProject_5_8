"""尝试读 Seven 网格的顶点/LOD 数量，用于和 Max (Seven_Head 4526 verts) 对比。"""
import json

import unreal

MESH = '/Game/_MyTest/Mouth/Mesh/Seven'
mesh = unreal.load_asset(MESH)
out = {}

for name in ('get_num_lods', 'get_num_sections', 'get_num_vertices', 'get_num_triangles',
             'get_editor_property', 'get_skeletal_mesh_description', 'get_lod_count'):
    out[name] = hasattr(mesh, name)

for attr in ('lod_settings', 'num_lods', 'lod_count', 'extended_bounds', 'bounding_box'):
    try:
        out['prop_' + attr] = str(mesh.get_editor_property(attr))[:300]
    except Exception as exc:
        out['prop_' + attr] = 'ERR'

try:
    out['num_lods_call'] = mesh.get_num_lods()
except Exception as exc:
    out['num_lods_call'] = 'ERR %r' % exc

# SkeletalMesh 的 LOD 信息可能通过 mesh.get_editor_property('lod_info') 的替代 API
for m in ('get_lod_info', 'get_mesh_description', 'get_render_data'):
    if hasattr(mesh, m):
        try:
            out[m] = str(getattr(mesh, m)())
        except Exception as exc:
            out[m] = 'ERR %r' % exc

print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
