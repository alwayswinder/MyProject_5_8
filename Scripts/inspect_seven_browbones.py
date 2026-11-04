"""打印 UE Seven 骨架里眉部骨骼的父子与位置。"""
import json

import unreal

MESH = '/Game/_MyTest/Mouth/Mesh/Seven'


def main():
    mesh = unreal.load_asset(MESH)
    rig = unreal.new_object(unreal.RigHierarchy)
    rig.get_controller(True).import_bones_from_skeletal_mesh(
        mesh, 'None', True, True, False, False, False)
    keys = [k for k in rig.get_all_keys() if k.type == unreal.RigElementType.BONE]
    names = {str(k.name): k for k in keys}

    targets = ['head', 'yankuang_L', 'yankuang_R', 'meibu_L', 'meibu_R'] + \
              ['Bone%03d' % i for i in range(1, 24)]
    out = {}
    rows = []
    for n in targets:
        k = names.get(n)
        if not k:
            rows.append({'bone': n, 'missing': True})
            continue
        p = rig.get_first_parent(k)
        t = rig.get_global_transform(k)
        rows.append({
            'bone': n,
            'parent': str(p.name) if p else '',
            'pos': [round(t.translation.x, 3), round(t.translation.y, 3), round(t.translation.z, 3)],
        })
    out['bones'] = rows
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
