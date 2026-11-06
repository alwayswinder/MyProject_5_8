"""把上眼皮控制设为闭合，供截图验证穿模。"""
import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    inst = rig.get_debugged_control_rig()
    if inst is None:
        print('NO INSTANCE')
        return
    rh = inst.get_hierarchy()
    for c in ('CTRL_EYE_LID_R', 'CTRL_EYE_LID_L'):
        rh.set_control_value(unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=c),
                             rh.make_control_value_from_vector(unreal.Vector(0.0, 0.0, -10.0)))
    inst.execute('Forwards Solve')
    print('LID CLOSED')


if __name__ == '__main__':
    main()
