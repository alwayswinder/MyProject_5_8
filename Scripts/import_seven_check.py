"""把新导出的 FBX 导入到临时目录，检查骨架与网格。"""
import json

import unreal

FBX = 'D:/Download/Seven_v2.fbx'
DEST = '/Game/_MyTest/Temp'


def main():
    task = unreal.AssetImportTask()
    task.filename = FBX
    task.destination_path = DEST
    task.destination_name = 'SevenCheck'
    task.replace_existing = True
    task.automated = True
    task.save = True
    try:
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        out = {'imported': [str(p) for p in task.imported_object_paths]}
    except Exception as exc:
        out = {'error': repr(exc)}

    assets = {}
    for path in ('/Game/_MyTest/Temp/SevenCheck', '/Game/_MyTest/Temp/SevenCheck_Skeleton'):
        try:
            a = unreal.load_asset(path)
            assets[path] = str(a) if a else 'None'
        except Exception as exc:
            assets[path] = 'ERR %r' % exc
    out['assets'] = assets
    try:
        sk = unreal.load_asset('/Game/_MyTest/Temp/SevenCheck_Skeleton')
        if sk:
            out['bone_count'] = len(sk.get_editor_property('bone_tree'))
    except Exception as exc:
        out['bone_count'] = 'ERR %r' % exc
    try:
        mesh = unreal.load_asset('/Game/_MyTest/Temp/SevenCheck')
        if mesh:
            out['morph_count'] = len(mesh.get_editor_property('morph_targets'))
    except Exception as exc:
        out['morph_count'] = 'ERR %r' % exc
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
