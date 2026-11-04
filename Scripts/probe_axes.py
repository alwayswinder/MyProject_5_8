"""探测 Seven 骨架眉毛链的世界位置与局部轴，用于决定 RigVM 偏移分量。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
BONES = ['head', 'yankuang_L', 'meibu_L', 'Bone007', 'Bone008', 'Bone009',
         'yankuang_R', 'meibu_R', 'Bone010', 'Bone011', 'Bone012']


def main():
    rig = unreal.load_asset(ASSET)
    h = rig.get_hierarchy()
    m = unreal.MathLibrary
    out = {}
    for b in BONES:
        k = unreal.RigElementKey(type=unreal.RigElementType.BONE, name=b)
        g = h.get_global_transform(k)
        rot = g.rotation.rotator()
        out[b] = {
            'world_t': [round(g.translation.x, 3), round(g.translation.y, 3), round(g.translation.z, 3)],
            'world_scale': [round(g.scale3d.x, 4), round(g.scale3d.y, 4), round(g.scale3d.z, 4)],
            'world_quat': [round(g.rotation.x, 4), round(g.rotation.y, 4), round(g.rotation.z, 4), round(g.rotation.w, 4)],
            'localX_world': [round(v, 3) for v in (m.get_forward_vector(rot).x, m.get_forward_vector(rot).y, m.get_forward_vector(rot).z)],
            'localY_world': [round(v, 3) for v in (m.get_right_vector(rot).x, m.get_right_vector(rot).y, m.get_right_vector(rot).z)],
            'localZ_world': [round(v, 3) for v in (m.get_up_vector(rot).x, m.get_up_vector(rot).y, m.get_up_vector(rot).z)],
        }
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
