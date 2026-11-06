"""UE 端点：骨骼世界位置 + 四元数。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
BONES = ['Bone027', 'Bone028', 'Bone029', 'Bone036', 'Bone037', 'Bone038']


def main():
    rig = unreal.load_asset(ASSET)
    inst = rig.create_control_rig()
    inst.request_init()
    rh = inst.get_hierarchy()

    def snap(values):
        for c, v in values.items():
            rh.set_control_value(unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=c),
                                 rh.make_control_value_from_vector(unreal.Vector(*v)))
        inst.execute('Forwards Solve')
        res = {}
        for b in BONES:
            t = rh.get_global_transform(unreal.RigElementKey(type=unreal.RigElementType.BONE, name=b))
            res[b] = {
                'p': [round(t.translation.x, 4), round(t.translation.y, 4), round(t.translation.z, 4)],
                'q': [round(t.rotation.x, 4), round(t.rotation.y, 4), round(t.rotation.z, 4), round(t.rotation.w, 4)],
            }
        return res

    out = {
        'base': snap({}),
        'endpoint': snap({'CTRL_EYE_LID_R': (0, 0, -10), 'CTRL_EYE_LID_L': (0, 0, -10)}),
    }
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
