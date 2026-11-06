"""尝试读取 UE 网格的顶点数/导入信息。"""
import json

import unreal

MESH = '/Game/_MyTest/Mouth/Mesh/Seven'


def main():
    mesh = unreal.load_asset(MESH)
    out = {}
    for prop in ('skeletal_mesh_description', 'mesh_description', 'render_data', 'lod_settings',
                 'imported_bounds', 'skeleton', 'morph_targets', 'physics_asset', 'asset_import_data'):
        try:
            v = mesh.get_editor_property(prop)
            out[prop] = str(v)[:120]
        except Exception as exc:
            out[prop] = 'ERR'
    try:
        sk = mesh.get_editor_property('skeleton')
        out['skeleton_bone_count'] = len(sk.get_editor_property('bone_tree'))
    except Exception as exc:
        out['skeleton_bone_count'] = 'ERR %r' % exc
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
