import unreal
rig = unreal.load_asset("/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders")
h = rig.get_hierarchy()
k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name="CTRL_BROW_POINT")
off = h.get_global_control_offset_transform(k)
rot = off.rotation
print("rotation:", rot)
print("forward(X):", unreal.MathLibrary.get_forward_vector(rot))
print("right(Y)  :", unreal.MathLibrary.get_right_vector(rot))
print("up(Z)     :", unreal.MathLibrary.get_up_vector(rot))
print("gizmo scale:", off.scale3d)
