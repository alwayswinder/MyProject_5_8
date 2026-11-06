"""UE 网格顶点数与骨骼权重信息。"""
import json

import unreal

MESH = '/Game/_MyTest/Mouth/Mesh/Seven'


def main():
    mesh = unreal.load_asset(MESH)
    out = {}
    try:
        out['lod_count'] = mesh.get_num_lods()
    except Exception as exc:
        out['lod_count'] = 'ERR %r' % exc
    try:
        out['num_verts_lod0'] = mesh.get_num_vertices(0)
    except Exception as exc:
        out['num_verts_lod0'] = 'ERR %r' % exc
    try:
        out['num_triangles_lod0'] = mesh.get_num_triangles(0)
    except Exception as exc:
        out['num_triangles_lod0'] = 'ERR %r' % exc
    try:
        out['skeleton_bones'] = len(mesh.get_skeleton().get_editor_property('bone_tree'))
    except Exception as exc:
        out['skeleton_bones'] = 'ERR %r' % exc
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
