"""选中 16 个眼周点控，配合截图。"""
import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
NAMES = [
    'CTRL_EYE_R_CORNER_IN', 'CTRL_EYE_R_UP_IN', 'CTRL_EYE_R_UP_MID', 'CTRL_EYE_R_UP_OUT',
    'CTRL_EYE_R_CORNER_OUT', 'CTRL_EYE_R_DOWN_OUT', 'CTRL_EYE_R_DOWN_MID', 'CTRL_EYE_R_DOWN_IN',
    'CTRL_EYE_L_CORNER_IN', 'CTRL_EYE_L_UP_IN', 'CTRL_EYE_L_UP_MID', 'CTRL_EYE_L_UP_OUT',
    'CTRL_EYE_L_CORNER_OUT', 'CTRL_EYE_L_DOWN_OUT', 'CTRL_EYE_L_DOWN_MID', 'CTRL_EYE_L_DOWN_IN',
]


def main():
    rig = unreal.load_asset(ASSET)
    hc = rig.get_hierarchy_controller()
    hc.set_selection([unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=n) for n in NAMES])
    print('selected', len(NAMES))


if __name__ == '__main__':
    main()
