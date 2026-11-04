"""选中六个新控制并截屏前检查。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
CONTROLS = ['CTRL_BROW_L_IN', 'CTRL_BROW_L_MID', 'CTRL_BROW_L_OUT',
            'CTRL_BROW_R_IN', 'CTRL_BROW_R_MID', 'CTRL_BROW_R_OUT']


def main():
    rig = unreal.load_asset(ASSET)
    h = rig.get_hierarchy()
    hc = rig.get_hierarchy_controller()
    out = {'methods': [m for m in dir(hc) if 'select' in m.lower()]}
    keys = [unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=c) for c in CONTROLS]
    try:
        hc.set_selection(keys)
        out['set_selection'] = 'OK'
    except Exception as exc:
        out['set_selection'] = 'ERR %r' % exc
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
