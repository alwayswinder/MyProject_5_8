"""用 debugged 实例检查闭眼姿态。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
BONES = ['Bone027', 'Bone028', 'Bone029', 'Bone036', 'Bone037', 'Bone038']


def main():
    rig = unreal.load_asset(ASSET)
    inst = rig.get_debugged_control_rig()
    out = {'has_inst': inst is not None}
    if inst is None:
        print('JSONSTART')
        print(json.dumps(out))
        print('JSONEND')
        return
    rh = inst.get_hierarchy()
    out['bone_count'] = len([k for k in rh.get_all_keys() if k.type == unreal.RigElementType.BONE])
    for c in ('CTRL_EYE_LID_R', 'CTRL_EYE_LID_L'):
        rh.set_control_value(unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=c),
                             rh.make_control_value_from_vector(unreal.Vector(0.0, 0.0, -10.0)))
    inst.execute('Forwards Solve')
    poses = {}
    for b in BONES:
        t = rh.get_global_transform(unreal.RigElementKey(type=unreal.RigElementType.BONE, name=b))
        poses[b] = {
            'p': [round(t.translation.x, 3), round(t.translation.y, 3), round(t.translation.z, 3)],
            'q': [round(t.rotation.x, 3), round(t.rotation.y, 3), round(t.rotation.z, 3), round(t.rotation.w, 3)],
        }
    out['poses'] = poses
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
