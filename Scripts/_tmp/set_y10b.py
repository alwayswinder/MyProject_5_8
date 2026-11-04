import unreal
rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
h = rig.get_hierarchy()
k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_BROW_POINT')
h.set_control_value(k, unreal.RigHierarchy.make_control_value_from_vector2d(unreal.Vector2D(0.0, 10.0)))
print('Y=10')
