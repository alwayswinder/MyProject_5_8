"""打印眼周骨骼世界坐标与父级。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
NAMES = ['Bone027', 'Bone028', 'Bone029', 'Bone030', 'Bone031', 'Bone032',
         'Bone036', 'Bone037', 'Bone038', 'Bone039', 'Bone040', 'Bone041',
         'Bone042', 'Bone043', 'Bone044', 'Bone045']


def main():
    rig = unreal.load_asset(ASSET)
    h = rig.get_hierarchy()
    out = {}
    for n in NAMES:
        k = unreal.RigElementKey(type=unreal.RigElementType.BONE, name=n)
        t = h.get_global_transform(k).translation
        out[n] = {
            'w': [round(t.x, 3), round(t.y, 3), round(t.z, 3)],
            'parent': str(h.get_first_parent(k).name),
        }
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
