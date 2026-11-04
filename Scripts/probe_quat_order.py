"""验证四元数合成顺序：绕世界 X 轴旋转应改变 roll。"""
import json
import math

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    h = rig.get_hierarchy()
    out = {}
    for bone, deg in (('Bone010', -9.1674), ('Bone012', 10.313)):
        k = unreal.RigElementKey(type=unreal.RigElementType.BONE, name=bone)
        q0 = h.get_global_transform(k).rotation
        a = math.radians(deg) * 0.5
        delta = unreal.Quat(math.sin(a), 0.0, 0.0, math.cos(a))
        try:
            q1 = delta * q0
            out[bone + '_delta_q0'] = str(unreal.MathLibrary.quat_rotator(q1).__str__())
        except Exception as exc:
            out[bone + '_delta_q0'] = 'ERR %r' % exc
        try:
            q2 = q0 * delta
            out[bone + '_q0_delta'] = str(unreal.MathLibrary.quat_rotator(q2).__str__())
        except Exception as exc:
            out[bone + '_q0_delta'] = 'ERR %r' % exc
        out[bone + '_base'] = str(unreal.MathLibrary.quat_rotator(q0).__str__())
        out[bone + '_expected_roll'] = 90.0 + deg
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
