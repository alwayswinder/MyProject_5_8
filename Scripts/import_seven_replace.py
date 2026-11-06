"""用原 Seven.fbx 覆盖导入 Mesh/Seven（8 骨影响设置生效后）。"""
import json

import unreal

FBX = 'D:/Download/Seven.fbx'


def main():
    out = {}
    mesh_before = unreal.load_asset('/Game/_MyTest/Mouth/Mesh/Seven')
    sk_before = unreal.load_asset('/Game/_MyTest/Mouth/Mesh/Seven_Skeleton')
    out['before'] = {
        'mesh': str(mesh_before) if mesh_before else None,
        'bones': len(sk_before.get_editor_property('bone_tree')) if sk_before else None,
        'morphs': len(mesh_before.get_editor_property('morph_targets')) if mesh_before else None,
    }

    task = unreal.AssetImportTask()
    task.filename = FBX
    task.destination_path = '/Game/_MyTest/Mouth/Mesh'
    task.destination_name = 'Seven'
    task.replace_existing = True
    task.automated = True
    task.save = True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    out['imported'] = [str(p) for p in task.imported_object_paths]

    mesh_after = unreal.load_asset('/Game/_MyTest/Mouth/Mesh/Seven')
    sk_after = unreal.load_asset('/Game/_MyTest/Mouth/Mesh/Seven_Skeleton')
    out['after'] = {
        'mesh': str(mesh_after) if mesh_after else None,
        'bones': len(sk_after.get_editor_property('bone_tree')) if sk_after else None,
        'morphs': len(mesh_after.get_editor_property('morph_targets')) if mesh_after else None,
    }
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
