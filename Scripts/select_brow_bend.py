"""选中右眉弯曲控制，配合截图。"""
import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    hc = rig.get_hierarchy_controller()
    k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_R_BEND')
    hc.set_selection([k])
    print('selected')


if __name__ == '__main__':
    main()
