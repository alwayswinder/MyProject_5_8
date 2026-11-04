"""列出 Seven 骨架里的眉部骨骼。"""
import json

import unreal

MESH = '/Game/_MyTest/Mouth/Mesh/Seven'


def main():
    mesh = unreal.load_asset(MESH)
    sk = mesh.get_editor_property('skeleton')
    out = {'mesh': MESH, 'skeleton': sk.get_path_name() if sk else None}
    names = []
    if sk:
        count = unreal.SkeletalMeshEditorSubsystem  # noqa
        try:
            ref_skel = sk.get_editor_property('skeleton')
        except Exception:
            pass
        try:
            ni = sk.get_editor_property('bone_tree')
        except Exception:
            ni = None
        names = [str(n) for n in (sk.get_editor_property('bone_tree') or [])]
    out['bone_tree_raw'] = names

    # 通过 skeleton 的 reference skeleton 取名字
    try:
        ref = unreal.load_asset(sk.get_path_name())
        rig = unreal.new_object(unreal.RigHierarchy)
        rig.get_controller(True).import_bones_from_skeletal_mesh(
            mesh, 'None', True, True, False, False, False)
        keys = [k for k in rig.get_all_keys()]
        bones = sorted(str(k.name) for k in keys if k.type == unreal.RigElementType.BONE)
        out['bone_count'] = len(bones)
        out['bones'] = bones
    except Exception as exc:
        out['import_err'] = repr(exc)

    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
