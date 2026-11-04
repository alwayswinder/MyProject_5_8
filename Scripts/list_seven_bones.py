"""列出 Seven 骨架的全部骨骼名，用于确认能否补全眉毛链 Bone007-013。"""
import json

import unreal

MESH = '/Game/_MyTest/Mouth/Mesh/Seven'


def main():
    mesh = unreal.load_asset(MESH)
    skel = mesh.get_editor_property('skeleton')
    names = []
    try:
        names = [str(skel.get_reference_name(n)) for n in skel.get_editor_property('bone_names')]
    except Exception as exc:
        names = ['<%r>' % exc]
    if not names or names[0].startswith('<'):
        ref = unreal.new_object(unreal.RigHierarchy)
        ref.get_controller(True).import_bones_from_skeletal_mesh(mesh, 'None', True, False, False, False, False)
        names = [str(k.name) for k in ref.get_all_keys() if k.type == unreal.RigElementType.BONE]

    out = {
        'mesh': MESH,
        'count': len(names),
        'all': names,
        'brow': [n for n in names if 'meibu' in n.lower() or n.lower().startswith('bone0')],
    }
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
