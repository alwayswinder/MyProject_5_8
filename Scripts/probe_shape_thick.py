"""试验 Box_thick 形状是否可用（改一个控制后截图对比，再改回）。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    h = rig.get_hierarchy()
    k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_R_BEND')
    st = h.get_control_settings(k)
    out = {'before': str(st.shape_name)}
    st.shape_name = 'Box_thick'
    h.set_control_settings(k, st)
    st2 = h.get_control_settings(k)
    out['after_set'] = str(st2.shape_name)
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
