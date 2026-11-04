"""探测 import_bones_from_skeletal_mesh 的签名与全量导入结果。"""
import json

import unreal

MESH = '/Game/_MyTest/Mouth/Mesh/Seven'


def main():
    out = {}
    fn = unreal.RigHierarchyController.import_bones_from_skeletal_mesh
    out['doc'] = str(unreal.RigHierarchyController.import_bones_from_skeletal_mesh.__doc__)
    out['help'] = str(unreal.RigHierarchyController.import_bones_from_skeletal_mesh)

    try:
        h = unreal.new_object(unreal.RigHierarchy)
        hc = h.get_controller(True)
        mesh = unreal.load_asset(MESH)
        r = hc.import_bones_from_skeletal_mesh(mesh, 'None', False, True, True, True, False)
        out['call1_ok'] = str(r)
        out['call1_bones'] = len([k for k in h.get_all_keys() if k.type == unreal.RigElementType.BONE])
    except Exception as exc:
        out['call1_err'] = repr(exc)

    try:
        h2 = unreal.new_object(unreal.RigHierarchy)
        hc2 = h2.get_controller(True)
        mesh = unreal.load_asset(MESH)
        r = hc2.import_bones_from_skeletal_mesh(mesh)
        out['call2_ok'] = str(r)
        out['call2_bones'] = len([k for k in h2.get_all_keys() if k.type == unreal.RigElementType.BONE])
    except Exception as exc:
        out['call2_err'] = repr(exc)

    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
