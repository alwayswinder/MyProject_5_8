"""把 debugged rig 的新点控设成夸张值，用于编辑器内视觉验证。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def key(name):
    return unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=name)


def main():
    rig = unreal.load_asset(ASSET)
    inst = rig.get_debugged_control_rig()
    out = {'has_instance': inst is not None}
    if inst is None:
        print('JSONSTART')
        print(json.dumps(out))
        print('JSONEND')
        return
    rh = inst.get_hierarchy()
    vals = {
        'CTRL_BROW_L_IN': (0.0, 0.0, 8.0),
        'CTRL_BROW_L_MID': (0.0, 0.0, -6.0),
        'CTRL_BROW_L_OUT': (0.0, 0.0, 4.0),
        'CTRL_BROW_R_IN': (0.0, 0.0, 8.0),
        'CTRL_BROW_R_MID': (0.0, 0.0, -6.0),
        'CTRL_BROW_R_OUT': (0.0, 0.0, 4.0),
    }
    for c, v in vals.items():
        try:
            rh.set_control_value(key(c), rh.make_control_value_from_vector(unreal.Vector(*v)))
            out[c] = list(v)
        except Exception as exc:
            out[c] = 'ERR %r' % exc
    inst.execute('Forwards Solve')
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False))
    print('JSONEND')


if __name__ == '__main__':
    main()
