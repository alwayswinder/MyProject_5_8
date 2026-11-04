"""探测 RigVMController 的链接 API 与形状库。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    out = {}
    rig = unreal.load_asset(ASSET)
    ctrl = rig.get_controller(rig.get_default_model())
    out['link_methods'] = [m for m in dir(ctrl) if 'link' in m.lower()]
    out['pin_methods'] = [m for m in dir(ctrl) if 'pin' in m.lower()]
    out['hierarchy_methods'] = [m for m in dir(rig.get_hierarchy()) if 'shape' in m.lower()]
    try:
        out['shape_lib'] = [m for m in dir(unreal.ControlRigShapeLibrary) if not m.startswith('_')]
    except Exception as exc:
        out['shape_lib'] = 'ERR %r' % exc
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
