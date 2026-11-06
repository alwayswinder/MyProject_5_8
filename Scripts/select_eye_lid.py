"""选中上眼皮控制，配合截图。"""
import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    hc = rig.get_hierarchy_controller()
    keys = [unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=n)
            for n in ('CTRL_EYE_LID_L', 'CTRL_EYE_LID_R')]
    hc.set_selection(keys)
    print('selected 2')


if __name__ == '__main__':
    main()
