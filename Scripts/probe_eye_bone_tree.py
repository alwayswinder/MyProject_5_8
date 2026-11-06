"""查看眼睑骨的父子关系（UE 层级）。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
NAMES = ['Bone027', 'Bone028', 'Bone029', 'Bone036', 'Bone037', 'Bone038', 'Eye_R', 'Eye_L', 'yankuang_R', 'yankuang_L']


def main():
    rig = unreal.load_asset(ASSET)
    h = rig.get_hierarchy()
    out = {}
    for n in NAMES:
        k = unreal.RigElementKey(type=unreal.RigElementType.BONE, name=n)
        kids = []
        try:
            kids = [str(c.name) for c in h.get_children(k)]
        except Exception as exc:
            kids = ['ERR %r' % exc]
        out[n] = {'parent': str(h.get_first_parent(k).name), 'children': kids}
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
