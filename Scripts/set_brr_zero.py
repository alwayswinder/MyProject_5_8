import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
rig = unreal.load_asset(ASSET)
inst = rig.get_debugged_control_rig()
h = inst.get_hierarchy()
k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_R')
h.set_control_value(k, h.make_control_value_from_vector(unreal.Vector(0.0, 0.0, 0.0)),
                    unreal.RigControlValueType.CURRENT, True)
print('set 0,0,0')
